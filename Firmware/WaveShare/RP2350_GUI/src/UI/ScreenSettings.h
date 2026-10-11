#ifndef LV_SCREENSETTINGS_H
#define LV_SCREENSETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

extern lv_obj_t * screensettings;
extern lv_obj_t * sw_savesettings;

void Setup_ScreenSettings(byte index, bool show);

void switch_to_dark(bool use_dark_mode);

bool GetDarkTheme(void);
bool GetButtonBeep(void);
bool GetHeartbeat(void);

void SetDarkTheme(bool datavalue);
void SetButtonBeep(bool datavalue);
void SetHeartbeat(bool datavalue);

int32_t GetNodeAdress(void);
int32_t GetMasterAdress(void);
int32_t GetIT2704Adress(void);
int32_t GetIT2704Channel(void);

void SetNodeAdress(byte datavalue);
void SetMasterAdress(byte datavalue);
void SetIT2704Adress(byte datavalue);
void SetIT2704Channel(byte datavalue);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENSETTINGS_H*/