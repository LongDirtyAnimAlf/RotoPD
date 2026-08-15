#include <Wire.h>
#include "./src/sensors/TouchDrvGT911.hpp"
#include "./src/CH32/WS_CH32_IO.h"

#define maxTouchPoints 5

#ifdef ARDUINO_ESP32S3_DEV
extern USBCDC USBSerial;
#endif

int16_t touch_last_x = 0, touch_last_y = 0;

static TouchDrvGT911 GT911;

static bool gt911_available = false;

static bool touch_swap_xy = false;
static int16_t touch_map_x1 = -1;
static int16_t touch_map_x2 = -1;
static int16_t touch_map_y1 = -1;
static int16_t touch_map_y2 = -1;

static int16_t touch_max_x = 0, touch_max_y = 0;
static int16_t touch_raw_x = 0, touch_raw_y = 0;

bool touch_init(int16_t w, int16_t h, uint8_t r)
{
  touch_max_x = w - 1;
  touch_max_y = h - 1;
  if (touch_map_x1 == -1)
  {
    switch (r)
    {
    case 0:
      touch_swap_xy = false;
      touch_map_x1 = touch_max_x;
      touch_map_x2 = 0;
      touch_map_y1 = touch_max_y;
      touch_map_y2 = 0;
      break;
    case 1:
      touch_swap_xy = true;
      touch_map_x1 = touch_max_x;
      touch_map_x2 = 0;
      touch_map_y1 = 0;
      touch_map_y2 = touch_max_y;
      break;
    case 2:
      touch_swap_xy = false;
      touch_map_x1 = 0;
      touch_map_x2 = touch_max_x;
      touch_map_y1 = 0;
      touch_map_y2 = touch_max_y;
      break;
    case 3:    
      touch_swap_xy = true;
      touch_map_x1 = 0;
      touch_map_x2 = touch_max_x;
      touch_map_y1 = touch_max_y;
      touch_map_y2 = 0;
      break;
    }
  }

  uint8_t gt911_i2c_addr = GT911_SLAVE_ADDRESS_L;

  GT911.setPins(-1, -1);
  if (GT911.begin(Wire, gt911_i2c_addr, WS_CH32_IO::DEFAULT_I2C_SDA, WS_CH32_IO::DEFAULT_I2C_SCL)) {
    USBSerial.print("GT911 initialized successfully at address 0x");
    USBSerial.println(gt911_i2c_addr, HEX);
    gt911_available = true;
  } else {
    USBSerial.print("Failed to initialize GT911 at address 0x");
    USBSerial.println(gt911_i2c_addr, HEX);
    gt911_available = false;
  }

  if (gt911_available)
  {
    GT911.setHomeButtonCallback([](void *user_data) {
      USBSerial.println("Home button pressed!");
    },
    NULL);
    GT911.setMaxTouchPoint(1); // max is 5
  }
  else
  {
    USBSerial.println("GT911 not found; running in LCD-only mode for ESP32-S3-LCD-4.");
  }

  return (gt911_available);
}

bool touch_has_signal()
{
  return (gt911_available);
}

void translate_touch_raw()
{
  if (touch_swap_xy)
  {
    touch_last_x = map(touch_raw_y, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_x, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
  else
  {
    touch_last_x = map(touch_raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
}

bool touch_touched(void)
{
  int16_t x[maxTouchPoints], y[maxTouchPoints];  
  uint8_t touched = GT911.getPoint(x, y, GT911.getSupportTouchPoint());
  if (touched > 0)
  {
    for (int i = 0; i < touched; ++i)
    {

      touch_raw_x = x[i];
      touch_raw_y = y[i];

      touch_last_x = touch_raw_x;
      touch_last_y = touch_raw_y;

      translate_touch_raw();
    }
    return (true);
  }
  return (false);
}

bool touch_released()
{
  return false;
}
