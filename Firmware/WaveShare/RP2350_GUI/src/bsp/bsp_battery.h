#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define BSP_BAT_ADC_PIN     41
#define BSP_BAT_CHRG_PIN    42
#define BSP_BAT_DONE_PIN    43

#ifdef __cplusplus
extern "C" {
#endif

void bsp_battery_init(void);
void bsp_battery_read(float *voltage, uint16_t *adc_raw);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __BSP_BATTERY_H__

