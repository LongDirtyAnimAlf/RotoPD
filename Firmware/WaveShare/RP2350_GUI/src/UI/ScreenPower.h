#ifndef LV_SCREENPOWER_H
#define LV_SCREENPOWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>
#include "./../../shared.h"

extern lv_obj_t * screenpower;

void Setup_ScreenPower(byte index);
void ScreenPowerAddData(word P, word E);
void ScreenPowerSetData(PRunDatas MAD);
byte ScreenPowerGetActive(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENPOWER_H*/