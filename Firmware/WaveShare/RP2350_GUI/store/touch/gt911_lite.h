
#ifndef GT911_LITE_H
#define GT911_LITE_H

#include "Arduino.h"
#include <Wire.h>

#define GT911_ADDR1 (uint8_t)0x5D
#define GT911_ADDR2 (uint8_t)0x14

#define GT911_RST_PIN 16
#define GT911_INT_PIN 17

#define GT911_POINT_INFO (uint16_t)0x814E
#define GT911_POINT_1    (uint16_t)0x814F
#define GT911_POINT_2    (uint16_t)0x8157
#define GT911_POINT_3    (uint16_t)0x815F
#define GT911_POINT_4    (uint16_t)0x8167
#define GT911_POINT_5    (uint16_t)0x816F
#define GT911_POINTS_REG {GT911_POINT_1, GT911_POINT_2, GT911_POINT_3, GT911_POINT_4, GT911_POINT_5}

#define GT911_CONFIG_REG    (uint16_t)0x8047
#define GT911_CONFIG_LEN    184
#define GT911_CHECKSUM_REG  (uint16_t)0x80FF
#define GT911_FRESH_REG     (uint16_t)0x8100

class TP_Point {
  public:
    TP_Point(void);
    TP_Point(uint8_t id, uint16_t x, uint16_t y, uint16_t size);

    bool operator==(TP_Point);
    bool operator!=(TP_Point);

    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t size;
};

class GT911_Lite {
  public:
    GT911_Lite();
    void begin(TwoWire *wire);
    void begin(TwoWire *wire, uint8_t sensitivity);
    void begin(TwoWire *wire, uint8_t sensitivity, uint16_t width, uint16_t height);
    bool read(void);

    // Flip reported coordinates 180° (for an upside-down panel mount).  maxX/maxY
    // are the touch coordinate ranges (controller resolution), so x→maxX-x etc.
    void setRotate180(bool on, uint16_t maxX, uint16_t maxY) {
        _rot180 = on; _rotMaxX = maxX; _rotMaxY = maxY;
    }

    uint8_t isLargeDetect;
    uint8_t touches = 0;
    bool isTouched = false;
    bool down = false;
    // Monotonic count of fresh frames (0x814E buffer-status seen set), bumped
    // every read() that consumes a frame — independent of whether coordinates
    // changed.  Lets callers measure the controller's true report rate.
    uint32_t frameCount = 0;
    // Anti-phantom: count of single-frame (unconfirmed) touch starts dropped —
    // an EPD-refresh transient produces one isolated frame then the controller
    // idles, so these are almost certainly phantoms, not fingers.
    uint32_t rejectedTouches = 0;
    TP_Point points[5];

    uint16_t x;
    uint16_t y;
    uint16_t size;

    uint16_t last_x;
    uint16_t last_y;
    uint16_t last_size;
    bool last_down;

  private:
    bool     _rot180  = false;
    uint16_t _rotMaxX = 0;
    uint16_t _rotMaxY = 0;

    void applyConfig(uint8_t sensitivity, bool setRes, uint16_t width, uint16_t height);
    TP_Point readPoint(uint8_t *data);
    void writeByteData(uint16_t reg, uint8_t val);
    uint8_t readByteData(uint16_t reg);
    void readBlockData(uint16_t reg, uint8_t *buf, uint8_t size);
    uint8_t addr;
    TwoWire *_wire;

    // Touch-confirmation state (anti-phantom — see read()).
    uint8_t  _consecTouch = 0;   // consecutive fresh frames currently reporting a touch
    uint32_t _lastFrameMs = 0;   // millis() of the last fresh frame
};

#endif // GT911_LITE_H
