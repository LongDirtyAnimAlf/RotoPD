#ifndef LV_SCREENCHART_H
#define LV_SCREENCHART_H

#include <lvgl.h>
#include "./../../shared.h"

extern lv_obj_t * screenchart;

#ifdef __cplusplus
extern "C" {
#endif

void Setup_ScreenChart(byte index);
void ScreenChartAddData(word V, word I);
void ScreenChartSetData(PRunDatas MAD);
void ScreenChartSetIT2704Data(PRunDatas MAD);
byte ScreenChartGetActive(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENCHART_H*/