#ifndef LV_SCREENSETTINGS_H
#define LV_SCREENSETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

extern lv_obj_t * screensettings;

void Setup_ScreenSettings(byte index, bool show);

int32_t GetNodeAdress(void);
int32_t GetMasterAdress(void);
int32_t GetIT2704Adress(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENSETTINGS_H*/