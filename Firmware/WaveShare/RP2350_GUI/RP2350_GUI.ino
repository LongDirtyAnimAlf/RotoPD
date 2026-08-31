#define RP2350_PSRAM_CS 47

#include <Arduino.h>
#include <lvgl.h>
#include "pico/stdlib.h"
#include "./src/bsp/bsp_i2c.h"

#include "./src/lvgl/lv_port/lv_port_disp.h"
#include "./src/lvgl/lv_port/lv_port_indev.h"

#include "ui.h"
#include "shared.h"
#include "comms.h"

#define LVGL_TICK_PERIOD_MS 5

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PRESSED;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void setup() {
  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  bsp_i2c_init();
  Serial.println("RP2350. I2C init.");
  lv_init();
  Serial.println("RP2350. LVGL init.");
  lv_port_disp_init();
  Serial.println("RP2350. LVGL port init.");

  Serial.println("RP2350. LVGL ticker init.");
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time
  //add_repeating_timer_ms(LVGL_TICK_PERIOD_MS, lvgl_timer_cb, NULL, &lvgl_timer);

  Serial.println("RP2350. Touch init.");
  touch_init(480, 480, 0); // rotation will be handled by lvgl
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); //Touchpad should have POINTER type
  lv_indev_set_read_cb(indev, my_touchpad_read);
  
  CreateBaseScreen(NULL);
  if (screenbase != NULL)
  {
    Serial.println("RP2350. BaseScreen assigned.");
    lv_screen_load(screenbase);
    Setup_ScreenLogger(0,false);
    if (screenlogger != NULL) Serial.println("RP2350. Screenlogger assigned.");
    Info_Add("GUI. Init GUI.");      
    Setup_Screen3(0,false);
    if (screen3 != NULL) Serial.println("RP2350. Screen3 assigned.");
    Setup_Screen1(0);
    if (screen1 != NULL) Serial.println("RP2350. Screen1 assigned.");
  }

  //Screen1SetData(NULL);
  
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Serial.printf("LVGL heap: used %u / total %u, max used %u\n",
       mon.total_size - mon.free_size,
       mon.total_size,
       mon.max_used);

  // Internal SRAM heap
  Serial.printf("Internal RAM  free: %u bytes\n", rp2040.getFreeHeap());

  // PSRAM heap
  Serial.printf("PSRAM         free: %u bytes\n", rp2040.getFreePSRAMHeap());
  Serial.printf("PSRAM         total: %u bytes\n", rp2040.getTotalPSRAMHeap());
  Serial.printf("PSRAM         size : %u bytes\n", rp2040.getPSRAMSize());

  Serial.println("Init RP2350 ready.");
}

void loop()
{
  static uint32_t startTime = millis();
  static uint32_t counts = 0;

  if ((millis() - startTime) >= 1000UL)
  {
    startTime = millis();
    //Serial.println("Looping");

    counts++;

    if (counts==20)
    {
      Serial.println("Stop all");
      lv_anim_delete_all();                 // stop all animations
      lv_timer_enable(false);               // temporarily disable all LVGL timers
    }
  }
  lv_timer_handler_run_in_period(LVGL_TICK_PERIOD_MS);
}
