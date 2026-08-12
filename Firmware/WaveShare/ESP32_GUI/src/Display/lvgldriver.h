#ifndef LV_DRIVER_H
#define LV_DRIVER_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void lv_screen_init(void * gfx, word W, word H);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRIVER_H*/