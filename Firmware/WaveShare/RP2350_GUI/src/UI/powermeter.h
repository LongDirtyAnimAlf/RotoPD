#ifndef LV_POWERMETER_H
#define LV_POWERMETER_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void gauge_set_power(int32_t P);
void gauge_set_energy(int32_t E);
void gauge_set_voltage(int16_t V);
void gauge_set_current(int16_t I);
void create_3d_gauge(lv_obj_t * parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_POWERMETER_H*/