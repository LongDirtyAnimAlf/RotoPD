
#include "lv_port_disp.h"
#include <lvgl.h>
#include <stdbool.h>
#include "./../../bsp/pio_rgb.h"
#include "./../../bsp/bsp_st7701.h"
//#include "rp_pico_alloc.h"

#define MY_DISP_HOR_RES (480)
#define MY_DISP_VER_RES (320)

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

#define LVGL_DRAW_BUF_LINES  10 // number of display lines in each draw buffer in partial mode

//#define DRAW_BUF_SIZE (MY_DISP_HOR_RES * (MY_DISP_VER_RES / LVGL_DRAW_BUF_LINES) * BYTE_PER_PIXEL)
#define DRAW_BUF_SIZE (MY_DISP_HOR_RES * LVGL_DRAW_BUF_LINES * BYTE_PER_PIXEL)

static bsp_display_interface_t *display_if = NULL;

static lv_display_t *disp_drv = NULL;                         /*Descriptor of a display driver*/

void disp_flush(lv_display_t * drv, const lv_area_t * area, uint8_t * color_p)
{
    bsp_display_interface_t *display_if = (bsp_display_interface_t *)lv_display_get_user_data(drv);

    bsp_display_area_t display_area = {
        .x1 = (uint16_t)area->x1,
        .y1 = (uint16_t)area->y1,
        .x2 = (uint16_t)area->x2,
        .y2 = (uint16_t)area->y2
    };

    uint16_t * buf16 = (uint16_t *)color_p; // Let's say it's a 16 bit (RGB565) display

    display_if->flush_dma(&display_area, buf16);
    

    if (lv_display_flush_is_last(drv))
    {
        //gfxdisplay->flush();
        display_if->flush_dma(NULL, NULL);        
    }

    lv_display_flush_ready(drv);
}

void disp_flush_done(void)
{
    lv_display_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    pio_rgb_info_t rgb_info;// = {0};

    rgb_info.width = MY_DISP_HOR_RES;
    rgb_info.height = MY_DISP_VER_RES;
    //rgb_info.transfer_size = MY_DISP_HOR_RES * MY_DISP_VER_RES;
    //rgb_info.transfer_size = MY_DISP_HOR_RES * LVGL_DRAW_BUF_LINES;
    rgb_info.transfer_size = 0;
    rgb_info.pclk_freq = BSP_LCD_PCLK_FREQ;
    rgb_info.mode.double_buffer = false;
    rgb_info.mode.enabled_transfer = false;
    rgb_info.mode.enabled_psram = false;
    //rgb_info.framebuffer1 = rp_mem_malloc(MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL);
    rgb_info.framebuffer1 = (uint16_t *)malloc(MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL);
    //rgb_info.framebuffer1 = malloc(MY_DISP_HOR_RES * MY_DISP_VER_RES * sizeof(uint16_t));

    //rgb_info.framebuffer1 = NULL;
    rgb_info.framebuffer2 = NULL;
    //rgb_info.transfer_buffer1 = NULL;
    //rgb_info.transfer_buffer2 = NULL;    

    rgb_info.dma_flush_done_cb = NULL;

    bsp_display_info_t display_info;//  = {0};
    display_info.width = MY_DISP_HOR_RES;
    display_info.height = MY_DISP_VER_RES;
    display_info.brightness = 80;
    display_info.dma_flush_done_cb = NULL;
    display_info.user_data = &rgb_info;

    bsp_display_new_st7701(&display_if, &display_info);
    display_if->init();
    
    static uint8_t buf_data[DRAW_BUF_SIZE] __attribute__((aligned(4))); // 4-byte aligned for performance

    disp_drv = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_color_format(disp_drv, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp_drv, disp_flush);
    lv_display_set_buffers(disp_drv, buf_data, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(disp_drv, display_if);

    //lv_disp_drv_register(&disp_drv);
}
