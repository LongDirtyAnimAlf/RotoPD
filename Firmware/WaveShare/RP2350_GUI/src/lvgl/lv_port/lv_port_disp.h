#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize low level display driver */
void lv_port_disp_init(void);
void lv_screen_init(uint16_t W, uint16_t H);
uint32_t lv_port_get_buffer_heap_usage(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/
