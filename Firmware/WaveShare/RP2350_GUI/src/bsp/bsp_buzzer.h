#ifndef __BSP_LCD_BUZZER_H__
#define __BSP_LCD_BUZZER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#define BSP_BUZZER_PIN    1

#ifdef __cplusplus
extern "C" {
#endif


void bsp_buzzer_init(void);
void bsp_buzzer_enable(bool enable);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //__BSP_LCD_BUZZER_H__
