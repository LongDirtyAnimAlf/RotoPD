#include "Arduino.h"
#include "gt911_lite.h"

#include <Wire.h>
#include <initializer_list>

GT911_Lite::GT911_Lite() : addr(0) {
}

void GT911_Lite::begin(TwoWire *wire) {
  _wire = wire;
  addr = 0;

  /*
  gpio_put(GT911_RST_PIN, 1);
  sleep_ms(50);
  gpio_put(GT911_RST_PIN, 0);
  sleep_ms(50);
  gpio_put(GT911_RST_PIN, 1);
  sleep_ms(250);
  */

  // Probe with a REAL register read, not an empty address-only
  // transaction: some core/chip combinations NACK a zero-length write
  // (seen on ESP32-S3 + LilyGo T5 S3 — the chip is there, answers
  // register reads, but "detects" as absent), which left addr = 0 and
  // made every later read() silently talk to address 0.
  for (uint8_t a : { GT911_ADDR1, GT911_ADDR2 }) {
    _wire->beginTransmission(a);
    _wire->write(highByte(GT911_POINT_INFO));
    _wire->write(lowByte(GT911_POINT_INFO));
    if (_wire->endTransmission(false) != 0) continue;
    if (_wire->requestFrom(a, (uint8_t)1) != 1) continue;
    _wire->read();
    addr = a;
    break;
  }
  if (addr == GT911_ADDR1)      Serial.println("GT911 detected at ADDR1.");
  else if (addr == GT911_ADDR2) Serial.println("GT911 detected at ADDR2.");
  else                          Serial.println("GT911 not detected.");
}

void GT911_Lite::begin(TwoWire *wire, uint8_t sensitivity) {
  begin(wire);
  if (addr == 0) return;
  applyConfig(sensitivity, false, 0, 0);
}

void GT911_Lite::begin(TwoWire *wire, uint8_t sensitivity, uint16_t width, uint16_t height) {
  begin(wire);
  if (addr == 0) return;
  applyConfig(sensitivity, true, width, height);
}

void GT911_Lite::applyConfig(uint8_t sensitivity, bool setRes, uint16_t width, uint16_t height) {
  if (setRes && (width == 0 || height == 0)) {
    Serial.println("applyConfig: width/height must be non-zero, skipping");
    return;
  }

  uint8_t cfg[GT911_CONFIG_LEN];
  const uint8_t RCHUNK = 32;
  for (uint8_t off = 0; off < GT911_CONFIG_LEN; off += RCHUNK) {
    uint8_t n = (GT911_CONFIG_LEN - off < RCHUNK) ? (GT911_CONFIG_LEN - off) : RCHUNK;
    readBlockData(GT911_CONFIG_REG + off, cfg + off, n);
  }
  uint8_t storedChecksum = readByteData(GT911_CHECKSUM_REG);

  uint8_t computed = 0;
  for (uint16_t i = 0; i < GT911_CONFIG_LEN; i++) computed += cfg[i];
  if ((uint8_t)(computed + storedChecksum) != 0) {
    Serial.print("applyConfig: existing config checksum mismatch (stored=0x");
    Serial.print(storedChecksum, HEX);
    Serial.print(" expected=0x");
    Serial.print((uint8_t)(0 - computed), HEX);
    Serial.println(") - aborting, chip not modified");
    return;
  }

  uint8_t  oldTouch = cfg[0x0C];
  uint8_t  oldLeave = cfg[0x0D];
  uint16_t oldW     = cfg[0x01] | ((uint16_t)cfg[0x02] << 8);
  uint16_t oldH     = cfg[0x03] | ((uint16_t)cfg[0x04] << 8);

  uint8_t newLeave = sensitivity;
  uint8_t newTouch = sensitivity + 20;

  bool changed = false;

  if (newLeave != oldLeave) {
    Serial.print("Screen_Leave_Level: ");
    Serial.print(oldLeave);
    Serial.print(" -> ");
    Serial.println(newLeave);
    cfg[0x0D] = newLeave;
    changed = true;
  }
  if (newTouch != oldTouch) {
    Serial.print("Screen_Touch_Level: ");
    Serial.print(oldTouch);
    Serial.print(" -> ");
    Serial.println(newTouch);
    cfg[0x0C] = newTouch;
    changed = true;
  }
  if (setRes) {
    if (width != oldW) {
      Serial.print("X_Output_Max: ");
      Serial.print(oldW);
      Serial.print(" -> ");
      Serial.println(width);
      cfg[0x01] = width  & 0xFF;
      cfg[0x02] = width  >> 8;
      changed = true;
    }
    if (height != oldH) {
      Serial.print("Y_Output_Max: ");
      Serial.print(oldH);
      Serial.print(" -> ");
      Serial.println(height);
      cfg[0x03] = height & 0xFF;
      cfg[0x04] = height >> 8;
      changed = true;
    }
  }

  if (!changed) return;

  uint8_t sum = 0;
  for (uint16_t i = 0; i < GT911_CONFIG_LEN; i++) sum += cfg[i];
  uint8_t checksum = (uint8_t)(0 - sum);

  const uint8_t WCHUNK = 30;
  for (uint16_t off = 0; off < GT911_CONFIG_LEN; off += WCHUNK) {
    uint16_t n = (GT911_CONFIG_LEN - off < WCHUNK) ? (GT911_CONFIG_LEN - off) : WCHUNK;
    uint16_t reg = GT911_CONFIG_REG + off;
    _wire->beginTransmission(addr);
    _wire->write(highByte(reg));
    _wire->write(lowByte(reg));
    for (uint16_t i = 0; i < n; i++) _wire->write(cfg[off + i]);
    if (_wire->endTransmission() != 0) {
      Serial.println("applyConfig: I2C write failed mid-config, aborting");
      return;
    }
  }
  writeByteData(GT911_CHECKSUM_REG, checksum);
  writeByteData(GT911_FRESH_REG, 0x01);

  delay(200);
}


// ── Touch confirmation (anti-phantom) ────────────────────────────────────────
// An EPD-refresh voltage transient can couple into the capacitive sensor and
// make the GT911 emit a SINGLE isolated "touch" frame, after which the
// controller idles.  A real finger streams a fresh frame every ~20ms (≈48Hz)
// for as long as it is down (verified on hardware: real hold ≈48 frames/s,
// phantom = exactly 1 frame).  So we WITHHOLD a touch until it has been seen on
// GT_CONFIRM_FRAMES consecutive frames — a phantom (one frame) is never reported
// as a down anywhere.  The run restarts after a gap (a real touch never pauses
// that long between frames) so isolated phantoms can't accumulate into a false
// confirmation — but only while not already down, so a main-loop stall can't
// drop an established touch.
static const uint8_t  GT_CONFIRM_FRAMES   = 2;    // frames needed to trust a touch
static const uint32_t GT_FRAME_GAP_MAX_MS = 40;   // >this gap = start a new run

bool GT911_Lite::read() {
  uint8_t pointInfo = readByteData(GT911_POINT_INFO);
  if (!((pointInfo >> 7) & 1)) {
    return false;                       // no fresh frame ready
  }

  writeByteData(GT911_POINT_INFO, 0);
  frameCount++;                         // a fresh frame was consumed

  uint32_t now        = millis();
  uint8_t  rawTouches = pointInfo & 0x0F;

  // Restart the confirmation run if the controller had been quiet — only while
  // not already confirmed-down (so a stall can't tear down a real touch).
  if (!last_down && (now - _lastFrameMs) > GT_FRAME_GAP_MAX_MS) {
    if (_consecTouch > 0 && _consecTouch < GT_CONFIRM_FRAMES) rejectedTouches++;
    _consecTouch = 0;
  }
  _lastFrameMs = now;

  if (rawTouches > 0) {
    if (_consecTouch < 255) _consecTouch++;
  } else {
    // Release frame.  A pending-but-unconfirmed candidate here was a phantom.
    if (_consecTouch > 0 && _consecTouch < GT_CONFIRM_FRAMES) rejectedTouches++;
    _consecTouch = 0;
  }

  bool confirmed = (_consecTouch >= GT_CONFIRM_FRAMES);

  touches       = rawTouches;           // raw controller count (unfiltered)
  isLargeDetect = (pointInfo >> 6) & 1;

  uint16_t pointRegs[] = GT911_POINTS_REG;
  for (uint8_t i = 0; i < rawTouches && i < 5; i++) {
    uint8_t data[8];
    readBlockData(pointRegs[i], data, 8);
    points[i] = readPoint(data);
  }

  // Expose the touch (down / isTouched / x / y) only once confirmed.  An
  // unconfirmed first frame — possibly a phantom — is held back.
  if (confirmed) {
    isTouched = true;
    down = true;
    x = points[0].x;
    y = points[0].y;
    if (_rot180) {
      x = (_rotMaxX > x) ? _rotMaxX - x : 0;
      y = (_rotMaxY > y) ? _rotMaxY - y : 0;
    }
    size = points[0].size;
  } else {
    isTouched = false;
    down = false;
    x = last_x;
    y = last_y;
    size = last_size;
  }

  bool changed = x != last_x || y != last_y || size != last_size || down != last_down;

  last_x = x;
  last_y = y;
  last_size = size;
  last_down = down;

  return changed;
}


TP_Point GT911_Lite::readPoint(uint8_t *data) {
  uint8_t id = data[0];
  uint16_t x = data[1] + (data[2] << 8);
  uint16_t y = data[3] + (data[4] << 8);
  uint16_t size = data[5] + (data[6] << 8);


  return TP_Point(id, x, y, size);
}

void GT911_Lite::writeByteData(uint16_t reg, uint8_t val) {
  _wire->beginTransmission(addr);
  _wire->write(highByte(reg));
  _wire->write(lowByte(reg));
  _wire->write(val);
  _wire->endTransmission();
}

uint8_t GT911_Lite::readByteData(uint16_t reg) {
  _wire->beginTransmission(addr);
  _wire->write(highByte(reg));
  _wire->write(lowByte(reg));
  _wire->endTransmission(false);  // repeated start

  if (_wire->requestFrom(addr, (uint8_t)1) != 1) {
    return 0;
  }
  return _wire->read();
}

void GT911_Lite::readBlockData(uint16_t reg, uint8_t *buf, uint8_t size) {
  _wire->beginTransmission(addr);
  _wire->write(highByte(reg));
  _wire->write(lowByte(reg));
  _wire->endTransmission(false);  // repeated start

  uint8_t received = _wire->requestFrom(addr, size);
  if (received != size) {
    return;
  }

  for (uint8_t i = 0; i < size && _wire->available(); i++) {
    buf[i] = _wire->read();
  }
}


// TP_Point implementation

TP_Point::TP_Point()
  : id(0), x(0), y(0), size(0) {
}

TP_Point::TP_Point(uint8_t _id, uint16_t _x, uint16_t _y, uint16_t _size)
  : id(_id), x(_x), y(_y), size(_size) {
}

bool TP_Point::operator==(TP_Point point) {
  return (point.x == x) && (point.y == y) && (point.size == size);
}

bool TP_Point::operator!=(TP_Point point) {
  return (point.x != x) || (point.y != y) || (point.size != size);
}
