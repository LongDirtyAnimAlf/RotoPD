#include <Arduino.h>
#include <lvgl.h>
#include "pico/stdlib.h"
#include "./src/bsp/bsp_buzzer.h"
#include "./src/bsp/bsp_battery.h"
#include "./src/bsp/bsp_i2c.h"

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

#include "ui.h"
#include "shared.h"

static struct repeating_timer lvgl_timer;

#define LVGL_TICK_PERIOD_MS 5

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

static bool lvgl_timer_cb(repeating_timer_t *t) {
  lv_tick_inc(LVGL_TICK_PERIOD_MS);   // period in ms
  return true;
}

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

void setup() {
  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  //stdio_init_all();
  sleep_ms(1000);
  //set_cpu_clock(260);

  //bsp_buzzer_init();
  bsp_battery_init();
  Serial.println("RP2350. Battery init.");
  bsp_i2c_init();
  Serial.println("RP2350. I2C init.");
  lv_init();
  Serial.println("RP2350. LVGL init.");
  lv_port_disp_init();
  Serial.println("RP2350. LVGL port init.");

  Serial.println("RP2350. LVGL ticker init.");
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time
  //add_repeating_timer_ms(LVGL_TICK_PERIOD_MS, lvgl_timer_cb, NULL, &lvgl_timer);
  
  CreateBaseScreen(NULL);
  if (screenbase != NULL)
  {
    Serial.println("RP2350. BaseScreen assigned.");
    lv_screen_load(screenbase);
    //Setup_ScreenLogger(0,false);
    if (screenlogger != NULL) Serial.println("RP2350. Screenlogger assigned.");
    //Info_Add("GUI. Init GUI.");      
    //Setup_Screen3(0,false);
    if (screen3 != NULL) Serial.println("RP2350. Screen3 assigned.");
    Setup_Screen1(NULL);
  }

  //Screen1SetData(NULL);
  

  Serial.println("Init RP2350 ready.");
}

void loop() {

  //bsp_buzzer_enable(true);
  //delay(500);
  //bsp_buzzer_enable(false);
  //delay(500);

  //float voltage;
  //uint16_t adc_raw;

  //bsp_battery_read(&voltage, &adc_raw);  
  //Serial.printf("%f.\r\n",voltage);

  //uint32_t task_delay_ms = lv_timer_handler_run_in_period(LVGL_TICK_PERIOD_MS);
  lv_timer_handler();
  sleep_ms(LVGL_TICK_PERIOD_MS);
}
