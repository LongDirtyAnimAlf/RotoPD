#ifndef LV_DRIVER_H
#define LV_DRIVER_H

#ifdef ARDUINO_ARCH_ESP32

#include <Arduino.h>

#define LCD_BL 1 // Backlight control pin for Waveshare ESP32-4inch

#ifdef __cplusplus
extern "C" {
#endif

void lv_screen_init(void * gfx, word W, word H);
void lv_screen_brightness(uint8_t b);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif /*LV_DRIVER_H*/