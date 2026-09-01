#include "Arduino.h"
#include "lv_port_disp.h"
#include <lvgl.h>
#include <stdbool.h>
#include "./../../bsp/pio_rgb.h"
#include "./../../bsp/bsp_st7701.h"
//#include "./../../bsp/rp_pico_alloc.h"

#define DOUBLE_BUFFER
#define RENDER_MODE_DIRECT
#define USE_PSRAM

#define MY_DISP_HOR_RES (480)
#define MY_DISP_VER_RES (480)
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
#define LVGL_DRAW_BUF_LINES  60 // number of display lines in each draw buffer in partial mode
#define DRAW_BUF_SIZE (MY_DISP_HOR_RES * LVGL_DRAW_BUF_LINES * BYTE_PER_PIXEL)
#define BOUNCE_BUFFER_SIZE ((MY_DISP_HOR_RES * LVGL_DRAW_BUF_LINES)) // number of display lines in each bounce buffer

#ifndef USE_PSRAM
#undef DOUBLE_BUFFER
#endif

// Prefer non-static pointers (or clear static carefully)
uint8_t *buf_data_1 = NULL;
uint8_t *buf_data_2 = NULL;

#ifdef USE_PSRAM

// Two transfer buffers always needed !!
static uint16_t transfer_buffer1[BOUNCE_BUFFER_SIZE];
static uint16_t transfer_buffer2[BOUNCE_BUFFER_SIZE];

// Real framebuffers
static uint8_t framebuffer1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] PSRAM;
#ifdef DOUBLE_BUFFER
static uint8_t framebuffer2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] PSRAM;
#endif

#else

// Real framebuffers
static uint8_t framebuffer1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] __attribute__((aligned(2)));;
#ifdef DOUBLE_BUFFER
static uint8_t framebuffer2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] __attribute__((aligned(2)));;
#endif

#endif

static bsp_display_interface_t *display_if = NULL;

static lv_display_t *disp_drv = NULL;                         /*Descriptor of a display driver*/

void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    #ifndef RENDER_MODE_DIRECT

    bsp_display_interface_t *display_if = (bsp_display_interface_t *)lv_display_get_user_data(disp);

    bsp_display_area_t display_area = {
        .x1 = (uint16_t)area->x1,
        .y1 = (uint16_t)area->y1,
        .x2 = (uint16_t)area->x2,
        .y2 = (uint16_t)area->y2
    };

    display_if->flush_dma(&display_area, (uint16_t *)px_map); // Let's say it's a 16 bit (RGB565) display

    //lv_disp_flush_ready(disp);

    if (lv_display_flush_is_last(disp))
    {
      //gfxdisplay->flush();
      display_if->flush_dma(NULL, NULL); 
      //pio_rgb_change_framebuffer();   
      //pio_rgb_change_framebuffer();    
    }
    else
    {
        lv_disp_flush_ready(disp);
    }
    
    #else

    if (lv_display_flush_is_last(disp))
    {
      display_if->flush_dma(NULL, NULL); 
    }
    else
    lv_display_flush_ready(disp_drv);


    #ifndef USE_PSRAM
    lv_disp_flush_ready(disp);
    #endif

    #endif
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
    rgb_info.pclk_freq = BSP_LCD_PCLK_FREQ;
    #ifdef DOUBLE_BUFFER
    rgb_info.mode.double_buffer = true;
    #else
    rgb_info.mode.double_buffer = false;
    #endif

    rgb_info.framebuffer1 = (uint16_t *)framebuffer1;
    #ifdef DOUBLE_BUFFER
    rgb_info.framebuffer2 = (uint16_t *)framebuffer2;
    #else
    rgb_info.framebuffer2 = NULL;
    #endif


    #ifdef USE_PSRAM

    int i;

    rgb_info.mode.enabled_psram = true;

    rgb_info.transfer_size = BOUNCE_BUFFER_SIZE;
    rgb_info.mode.enabled_transfer = true;
    
    rgb_info.dma_flush_done_cb = disp_flush_done;

    #else //USE_PSRAM
    rgb_info.mode.enabled_psram = false;

    rgb_info.transfer_size = 0;
    rgb_info.mode.enabled_transfer = false;

    rgb_info.dma_flush_done_cb = NULL;

    #endif //USE_PSRAM

    if (rgb_info.mode.enabled_transfer)
    {
        rgb_info.transfer_buffer1 = transfer_buffer1;
        rgb_info.transfer_buffer2 = transfer_buffer2;
    }    

    bsp_display_info_t display_info;//  = {0};
    display_info.width = MY_DISP_HOR_RES;
    display_info.height = MY_DISP_VER_RES;
    display_info.brightness = 80;
    display_info.dma_flush_done_cb = NULL;
    display_info.user_data = &rgb_info;

    bsp_display_new_st7701(&display_if, &display_info);
    display_if->init();

    #ifdef RENDER_MODE_DIRECT
    buf_data_2 = (uint8_t *)rgb_info.framebuffer1;
    buf_data_1 = (uint8_t *)rgb_info.framebuffer2;
    //static uint8_t * buf_data_2 = NULL;
    #else
    buf_data_1 = (uint8_t *)malloc(DRAW_BUF_SIZE);
    #ifdef DOUBLE_BUFFER
    //buf_data_2 = (uint8_t *)malloc(DRAW_BUF_SIZE);
    #endif

    #endif

    disp_drv = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    #ifdef RENDER_MODE_DIRECT
    lv_display_set_buffers(disp_drv, buf_data_1, buf_data_2, (MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL), LV_DISPLAY_RENDER_MODE_DIRECT);
    #else
    lv_display_set_buffers(disp_drv, buf_data_1, buf_data_2, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    //lv_display_set_buffers(disp_drv, buf_data_1, buf_data_2, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_FULL);    
    #endif

    lv_display_set_color_format(disp_drv, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp_drv, disp_flush);
    lv_display_set_user_data(disp_drv, display_if);
}
