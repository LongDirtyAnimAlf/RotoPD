#ifndef LV_SCREENPOWER_H
#define LV_SCREENPOWER_H

#include <lvgl.h>
#include "./../../shared.h"

extern lv_obj_t * screenpower;

#ifdef __cplusplus
extern "C" {
#endif

void Setup_ScreenPower(byte index);
void ScreenPowerAddEPData(dword E, dword P);
void ScreenPowerSetData(PMeasurementData MAD);
byte ScreenPowerGetActive(void);

void ScreenPowerSetVoltageData(word V);
void ScreenPowerSetCurrentData(word I);
void ScreenPowerSetPowerData(dword P);
void ScreenPowerSetEnergyData(dword E);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENPOWER_H*/