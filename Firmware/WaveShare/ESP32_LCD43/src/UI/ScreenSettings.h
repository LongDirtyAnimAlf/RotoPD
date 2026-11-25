#ifndef LV_SCREENSETTINGS_H
#define LV_SCREENSETTINGS_H

#include <lvgl.h>

extern lv_obj_t * screensettings;
extern lv_obj_t * sw_savesettings;

#ifdef __cplusplus
extern "C" {
#endif

void Setup_ScreenSettings(byte index, bool show);
void ScreenSettings_SetLocal(bool Local);

void ScreenSettings_SwitchToDark(bool use_dark_mode);

bool ScreenSettings_GetDarkTheme(void);
bool ScreenSettings_GetButtonBeep(void);
//bool GetHeartbeat(void);

void ScreenSettings_SetDarkTheme(bool datavalue);
void ScreenSettings_SetButtonBeep(bool datavalue);
//void SetHeartbeat(bool datavalue);

int32_t ScreenSettings_GetBoardNumber(void);
int32_t ScreenSettings_GetNodeAdress(void);
int32_t ScreenSettings_GetMasterAdress(void);
int32_t ScreenSettings_GetIT2704Adress(void);
int32_t ScreenSettings_GetIT2704Channel(void);

void ScreenSettings_SetBoardNumber(byte datavalue);
void ScreenSettings_SetNodeAdress(byte datavalue);
void ScreenSettings_SetMasterAdress(byte datavalue);
void ScreenSettings_SetIT2704Adress(byte datavalue);
void ScreenSettings_SetIT2704Channel(byte datavalue);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENSETTINGS_H*/