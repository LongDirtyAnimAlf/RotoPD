#include "Arduino.h"
#include "lv_port_disp.h"
#include <lvgl.h>
#include <stdbool.h>
#include "./../../bsp/pio_rgb.h"
#include "./../../bsp/bsp_st7701.h"

#define DOUBLE_BUFFER
#define RENDER_MODE_DIRECT
#define USE_PSRAM

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

#ifndef USE_PSRAM
// No room in SRAM for double buffers
#undef DOUBLE_BUFFER
// No room in SRAM for pingpong buffers
#define PINGPONG_BUF_LINES  0  // number of display lines in each transfer buffer in SRAM
#define LVGL_DRAW_BUF_LINES  8 // number of display lines in each draw buffer in partial mode
#else
#define PINGPONG_BUF_LINES  20 // number of display lines in each transfer buffer in SRAM
#define LVGL_DRAW_BUF_LINES  40 // number of display lines in each draw buffer in partial mode
#endif

static uint32_t usedheapbytes = 0;

// Prefer non-static pointers (or clear static carefully)
static uint8_t *buf_data_1 = NULL;
static uint8_t *buf_data_2 = NULL;

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

    #endif

    if (lv_display_flush_is_last(disp))
    {
        #ifdef DOUBLE_BUFFER
        // We are ready with this framebuffer
        // Tell the hardware and switch framebuffer
        display_if->flush_dma(NULL, NULL); 
        #endif
        #ifndef USE_PSRAM
        lv_disp_flush_ready(disp);
        #endif
    }
    else
    {
        lv_disp_flush_ready(disp);
    }
}

void disp_flush_done(void)
{
    lv_display_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    lv_screen_init(480, 480);
}

void lv_screen_init(uint16_t W, uint16_t H)
{
    usedheapbytes = 0;

    pio_rgb_info_t rgb_info;// = {0};

    rgb_info.width = W;
    rgb_info.height = H;
    rgb_info.pclk_freq = BSP_LCD_PCLK_FREQ;
    #ifdef DOUBLE_BUFFER
    rgb_info.mode.double_buffer = true;
    #else
    rgb_info.mode.double_buffer = false;
    #endif

    #ifdef USE_PSRAM

    rgb_info.mode.enabled_psram = true;
    
    rgb_info.framebuffer1 = (uint16_t *)pmalloc(W * H * BYTE_PER_PIXEL);
    #ifdef DOUBLE_BUFFER
    //rgb_info.framebuffer2 = (uint16_t *)framebuffer2;
    rgb_info.framebuffer2 = (uint16_t *)pmalloc(W * H * BYTE_PER_PIXEL);
    #else
    rgb_info.framebuffer2 = NULL;
    #endif
    
    rgb_info.dma_flush_done_cb = disp_flush_done;
   
    #else //USE_PSRAM

    rgb_info.mode.enabled_psram = false;

    rgb_info.framebuffer1 = (uint16_t *)malloc(W * H * BYTE_PER_PIXEL);
    usedheapbytes += (W * H * BYTE_PER_PIXEL);
    #ifdef DOUBLE_BUFFER
    rgb_info.framebuffer2 = (uint16_t *)malloc(W * H * BYTE_PER_PIXEL);
    usedheapbytes += (W * H * BYTE_PER_PIXEL);
    #else
    rgb_info.framebuffer2 = NULL;
    #endif

    rgb_info.dma_flush_done_cb = NULL;
    
    #endif //USE_PSRAM

    rgb_info.transfer_size = (W * PINGPONG_BUF_LINES);
    rgb_info.mode.enabled_transfer = (rgb_info.transfer_size != 0);

    if (rgb_info.mode.enabled_transfer)
    {
        rgb_info.transfer_buffer1 = (uint16_t *)malloc(rgb_info.transfer_size * BYTE_PER_PIXEL);
        rgb_info.transfer_buffer2 = (uint16_t *)malloc(rgb_info.transfer_size * BYTE_PER_PIXEL);
        usedheapbytes += (rgb_info.transfer_size * 2 * BYTE_PER_PIXEL);
    }    
    else
    {
        rgb_info.transfer_buffer1 = NULL;
        rgb_info.transfer_buffer2 = NULL;
    }

    bsp_display_info_t display_info;//  = {0};
    display_info.width = W;
    display_info.height = H;
    display_info.brightness = 80;
    display_info.dma_flush_done_cb = NULL;
    display_info.user_data = &rgb_info;

    bsp_display_new_st7701(&display_if, &display_info);
    display_if->init();

    #ifdef RENDER_MODE_DIRECT
    #ifdef DOUBLE_BUFFER
    // Buffers are swapped, so painting starts on free buffer
    // Nice trick ... ;-)
    buf_data_2 = (uint8_t *)rgb_info.framebuffer1;
    buf_data_1 = (uint8_t *)rgb_info.framebuffer2;
    #else
    buf_data_1 = (uint8_t *)rgb_info.framebuffer1;
    #endif
    #else
    // Only single buffer needed
    buf_data_1 = (uint8_t *)malloc(W * LVGL_DRAW_BUF_LINES * BYTE_PER_PIXEL);
    usedheapbytes += (W * LVGL_DRAW_BUF_LINES * BYTE_PER_PIXEL);
    #endif

    disp_drv = lv_display_create(W, H);
    #ifdef RENDER_MODE_DIRECT
    lv_display_set_buffers(disp_drv, buf_data_1, buf_data_2, (W * H * BYTE_PER_PIXEL), LV_DISPLAY_RENDER_MODE_DIRECT);
    #else
    lv_display_set_buffers(disp_drv, buf_data_1, buf_data_2, (W * LVGL_DRAW_BUF_LINES * BYTE_PER_PIXEL), LV_DISPLAY_RENDER_MODE_PARTIAL);
    #endif

    lv_display_set_color_format(disp_drv, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp_drv, disp_flush);
    lv_display_set_user_data(disp_drv, display_if);
}

uint32_t lv_port_get_buffer_heap_usage(void)
{
    return (usedheapbytes);
}
