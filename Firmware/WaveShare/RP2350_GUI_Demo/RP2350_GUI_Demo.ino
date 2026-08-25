#include <Arduino.h>
#include <lvgl.h>
#include "./src/lvgl/lv_port/lv_port_disp.h"
#include "./src/lvgl/lv_port/lv_port_indev.h"

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

static void btn_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * btn = lv_event_get_current_target_obj(e);

  if (btn != NULL)
  {
    if (code == LV_EVENT_CLICKED)
    {
      lv_obj_set_style_bg_color(btn,lv_palette_darken(LV_PALETTE_RED,4), LV_PART_MAIN);      
    }
  }
}

void setup() {
  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  Serial.println("RP2350. LVGL init.");
  lv_init();
  lv_port_disp_init();
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time

  Serial.println("RP2350. Touch init.");
  touch_init(480, 480, 0); // rotation will be handled by lvgl
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  Serial.println("RP2350. Screen init.");
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
  lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "TOUCH ME !!!!");

  Serial.println("RP2350. Init ready.");
}

void loop() {
  lv_timer_handler();
  sleep_ms(5);
}
