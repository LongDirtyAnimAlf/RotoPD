#ifndef LV_SCREEN3_H
#define LV_SCREEN3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

extern lv_obj_t * screen3;
extern lv_obj_t * getpdolistbutton;

void Setup_Screen3(byte index);
void Screen3SetPDO(
  uint8_t  index,          // 1-based (1–13)
  bool     valid,          // detect bit = 1
  bool     isEPR,          // true for index 8–13
  uint8_t  type,           // PDO_TYPE_FIXED / _PPS / _AVS
  uint16_t minVoltage_mV,  // 0 for Fixed; 3300 for PPS; 15000 for AVS
  uint16_t maxVoltage_mV,
  uint16_t maxCurrent_mA);  // Approximate upper bound of current range

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREEN3_H*/