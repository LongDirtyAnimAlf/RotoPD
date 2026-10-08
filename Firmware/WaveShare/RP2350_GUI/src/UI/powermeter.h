#ifndef LV_POWERMETER_H
#define LV_POWERMETER_H

#include <Arduino.h>
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void gauge_set_value(int32_t power);
void create_3d_gauge(lv_obj_t * parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_POWERMETER_H*/