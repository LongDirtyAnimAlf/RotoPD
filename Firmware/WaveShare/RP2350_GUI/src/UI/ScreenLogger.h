#ifndef LV_SCREENLOGGER_H
#define LV_SCREENLOGGER_H

#include <lvgl.h>
#include <stdio.h>
#include <stdarg.h>

extern lv_obj_t * screenlogger;

#ifdef __cplusplus
extern "C" {
#endif

void Setup_ScreenLogger(byte index, bool show);
void ScreenLogger_Add(const char *txt, bool newline);
int ScreenLogger_Add_Fmt(const char *format, ...);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SCREENLOGGER_H*/