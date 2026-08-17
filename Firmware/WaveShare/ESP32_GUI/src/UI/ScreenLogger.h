#ifndef LV_SCREENLOGGER_H
#define LV_SCREENLOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

extern lv_obj_t * screenlogger;

void Setup_ScreenLogger(byte index, bool show);
void ScreenLogger_Add(const char *txt, bool newline);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENLOGGER_H*/