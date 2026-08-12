#ifndef LV_SCREEN1_H
#define LV_SCREEN1_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

extern lv_obj_t * screen1;

extern lv_obj_t * testdischargebutton;
extern lv_obj_t * startdischargebutton;
extern lv_obj_t * testchargebutton;
extern lv_obj_t * startchargebutton;

extern lv_obj_t * zerocapacitybutton;
extern lv_obj_t * zeroenergybutton;
extern lv_obj_t * zerotimebutton;

void Setup_Screen1(byte index);
void Screen1SetData(PBatterySetting SET);
void Screen1AddVIData(word V, word I);
void Screen1AddEPData(dword E, dword P);
void Screen1AddVData(word V);
void Screen1AddIData(word I);
void Screen1AddEData(dword E);
void Screen1AddPData(dword P);
void Screen1AddTData(dword T);
void Screen1SetThresholdLedEnabled(TThresholdModes Mode, bool On);
void Screen1SetThresholdLed(TThresholdModes Mode, bool On);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREEN1_H*/