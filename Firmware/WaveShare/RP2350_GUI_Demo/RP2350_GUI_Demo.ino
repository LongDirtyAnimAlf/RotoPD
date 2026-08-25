#include <Arduino.h>
#include <lvgl.h>
#include "./src/lvgl/lv_port/lv_port_disp.h"

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

void setup() {
  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  lv_init();
  lv_port_disp_init();
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time
  
  lv_obj_t* screen = lv_obj_create(NULL);
  lv_screen_load(screen);

  //lv_obj_set_style_bg_color(screen,lv_palette_darken(LV_PALETTE_GREEN,4), LV_PART_MAIN);

  lv_obj_set_style_border_width(screen, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(screen, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // Red border
  lv_obj_set_style_border_opa(screen, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);    

  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
  lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  
  lv_obj_set_style_bg_color(label,lv_palette_darken(LV_PALETTE_GREEN,4), LV_PART_MAIN);

  lv_obj_t * obj;

  obj = lv_button_create(screen);
  lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_set_size(obj, lv_pct(25), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);      

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "Back");


  Serial.println("Init RP2350 ready.");
}

void loop() {
  lv_timer_handler();
  sleep_ms(5);
}
