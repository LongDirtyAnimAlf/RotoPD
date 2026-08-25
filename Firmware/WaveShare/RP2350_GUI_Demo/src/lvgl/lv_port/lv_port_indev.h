#ifndef LV_PORT_INDEV_TEMPL_H
#define LV_PORT_INDEV_TEMPL_H

#include <Arduino.h>

extern int16_t touch_last_x, touch_last_y;

#ifdef __cplusplus
extern "C" {
#endif

bool touch_init(int16_t w, int16_t h, uint8_t r);
bool touch_touched(void);
bool touch_has_signal(void);
bool touch_released(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_TEMPL_H*/
