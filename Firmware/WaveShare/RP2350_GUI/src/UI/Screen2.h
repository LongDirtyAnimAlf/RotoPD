#ifndef LV_SCREEN2_H
#define LV_SCREEN2_H

#include <lvgl.h>
#include "./../../shared.h"

extern lv_obj_t * screen2;

#ifdef __cplusplus
extern "C" {
#endif

void Setup_Screen2(byte index);
void Screen2AddData(word V, word I);
void Screen2SetData(PRunDatas MAD);
void Screen2SetIT2704Data(PRunDatas MAD);
byte Screen2GetActive(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREEN2_H*/