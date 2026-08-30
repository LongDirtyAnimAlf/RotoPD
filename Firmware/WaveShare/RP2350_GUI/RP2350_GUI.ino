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

  Serial.printf("PSRAM Size reported by core: %d\n", rp2040.getPSRAMSize());
  Serial.printf("Free PSRAM heap: %d\n", rp2040.getFreePSRAMHeap());

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
  }

  Screen1SetData(NULL);
  
  Serial.println("Init RP2350 ready.");
}

void loop()
{
  static uint32_t startTime = millis();
  if ((millis() - startTime) >= 1000UL)
  {
    startTime = millis();
    Serial.println("Looping");
  }
  lv_timer_handler_run_in_period(LVGL_TICK_PERIOD_MS);
}
