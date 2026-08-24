#include <stdio.h>
#include "pico/stdlib.h"

#include "./../../bsp/bsp_i2c.h"
#include "../lv_port/lv_port_disp.h"
//#include "../lv_port/lv_port_indev.h"
//#include "demos/lv_demos.h"

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "./../../bsp/bsp_qmi8658.h"
#include "./../../bsp/bsp_pcf85063.h"

#define LVGL_TICK_PERIOD_MS 10

lv_obj_t *label_time;
lv_obj_t *label_date;

lv_timer_t *pcf85063_timer = NULL;


lv_timer_t *qmi8658_timer = NULL;

void set_cpu_clock(uint32_t freq_Mhz)
{
    set_sys_clock_khz(freq_Mhz * 1000, true);
    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        freq_Mhz * 1000 * 1000,
        freq_Mhz * 1000 * 1000);
}

static bool repeating_lvgl_timer_cb(struct repeating_timer *t)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
    return true;
}

static void pcf85063_callback(lv_timer_t *timer)
{
    struct tm now_tm;
    bsp_pcf85063_get_time(&now_tm);
    lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    lv_label_set_text_fmt(label_date, "%04d-%02d-%02d", now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
}

void lvgl_pcf85063_ui_init(void)
{
    lv_obj_t *list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, lv_pct(70), lv_pct(70));
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *list_item = lv_list_add_btn(list, NULL, "time");
    label_time = lv_label_create(list_item);
    lv_label_set_text(label_time, "12:00:00");

    list_item = lv_list_add_btn(list, NULL, "date");
    label_date = lv_label_create(list_item);
    lv_label_set_text(label_date, "2024-12-01");

    pcf85063_timer = lv_timer_create(pcf85063_callback, 1000, NULL);
}
