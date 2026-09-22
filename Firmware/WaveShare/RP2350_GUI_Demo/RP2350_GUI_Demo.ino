#define RP2350_PSRAM_CS 47

#include <Arduino.h>

#include "Wire.h"

//#include "Adafruit_TinyUSB.h"

#include <lvgl.h>
#include "./src/lvgl/lv_port/lv_port_disp.h"
#include "./src/bsp/bsp_i2c.h"
#include "./src/touch/gt911_lite.h"

GT911_Lite tp;

//#define Serial USBSerial

// Callback that returns elapsed ms since boot
static uint32_t my_tick_get_cb(void) {
  return millis();
}

static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  bool changed = tp.read();  

  if (changed)
  {
    data->point.x = tp.last_x;
    data->point.y = tp.last_y;
  }

  //if (tp.isTouched)
  if (tp.down)
  {
    data->state = LV_INDEV_STATE_PRESSED;
    //Set the coordinates
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

  static bool color = false;

  if (btn != NULL)
  {
    if (code == LV_EVENT_CLICKED)
    {
      if (color)
        lv_obj_set_style_bg_color(btn,lv_palette_darken(LV_PALETTE_RED, 1), LV_PART_MAIN);      
      else
        lv_obj_set_style_bg_color(btn,lv_palette_darken(LV_PALETTE_GREEN, 1), LV_PART_MAIN);      

      color = !color;  
    }
  }
}

void setup() {
  Serial.begin();
  int cnt = 1500;     // Will wait for up to ~5 second for Serial to connect.
  while (!Serial && cnt--) {delay(1);}
  Serial.println("Starting RP2350 init.");

  Serial.printf("PSRAM Size reported by core: %d\n", rp2040.getPSRAMSize());
  Serial.printf("Free PSRAM heap: %d\n", rp2040.getFreePSRAMHeap());

  Serial.println("RP2350. Wire1 init.");
  Wire1.setSDA(BSP_I2C_SDA_PIN);
  Wire1.setSCL(BSP_I2C_SCL_PIN);
  //Wire1.setSDA(6);
  //Wire1.setSCL(7);
  Wire1.begin();

  Serial.println("RP2350. LVGL init.");
  lv_init();
  lv_port_disp_init();
  lv_tick_set_cb(my_tick_get_cb);  // Tell LVGL how to get the current time

  Serial.println("RP2350. Touch init.");

  //tp.begin(&Wire1);
  tp.begin(&Wire1,10,480,480);
  tp.setRotate180(true,480,480);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); //Touchpad should have POINTER type
  lv_indev_set_read_cb(indev, my_touchpad_read);

  Serial.println("RP2350. Screen init.");
  lv_obj_t* screen = lv_obj_create(NULL);
  lv_screen_load(screen);

  //lv_obj_set_style_bg_color(screen,lv_palette_darken(LV_PALETTE_GREEN,4), LV_PART_MAIN);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1a1a1a), 0);

  lv_obj_set_style_border_width(screen, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(screen, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // Red border
  lv_obj_set_style_border_opa(screen, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);    

  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
  lv_obj_align( label, LV_ALIGN_BOTTOM_MID, 0, -20 );

  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);  
  //lv_obj_set_style_bg_color(label,lv_palette_darken(LV_PALETTE_GREEN,4), LV_PART_MAIN);
  lv_obj_set_style_text_color(label, lv_color_hex(0xFF00FF), 0);

  lv_obj_t * obj;

  obj = lv_button_create(screen);
  lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_set_size(obj, lv_pct(80), lv_pct(50));
  lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);      
  lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "TOUCH ME !!!!");
  lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);  


  obj = lv_button_create(screen);
  //lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_align( obj, LV_ALIGN_BOTTOM_MID, 0, -80 );
  //lv_obj_align_to(obj, label, LV_ALIGN_CENTER, 0, 0 );
  //lv_obj_align_to(obj, label, LV_ALIGN_OUT_TOP_MID, 0, 0 );
  lv_obj_set_size(obj, lv_pct(80), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);      
  lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "TOUCH ME ALSO !!!!");
  lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  



  Serial.printf("CPU Frequency: %d MHz\r\n",rp2040.f_cpu() / 1000000);

  Serial.println("GUI. RP2350 SRAM memory info.");
  // Internal SRAM heap
  Serial.printf("SRAM free  : %u kb\r\n", (rp2040.getFreeHeap() / 1000));
  Serial.printf("SRAM buffer: %u kb\r\n", (lv_port_get_buffer_heap_usage() / 1000));
  Serial.printf("SRAM free after lvgl buffer malloc : %u kb\r\n", ( (rp2040.getFreeHeap() - lv_port_get_buffer_heap_usage()) / 1000) );

  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Serial.printf("LVGL heap after init: used %u / total %u, max used %u\r\n",
       mon.total_size - mon.free_size,
       mon.total_size,
       mon.max_used);

  Serial.printf("SRAM free after lvgl buffer malloc : %u kb\r\n", ( (rp2040.getFreeHeap() - lv_port_get_buffer_heap_usage() - mon.total_size) / 1000) );

  Serial.println("GUI. RP2350 PSRAM memory info.");
  // PSRAM heap
  Serial.printf("PSRAM free  : %u bytes\r\n", rp2040.getFreePSRAMHeap());
  Serial.printf("PSRAM total : %u bytes\r\n", rp2040.getTotalPSRAMHeap());
  Serial.printf("PSRAM size  : %u bytes\r\n", rp2040.getPSRAMSize());

  Serial.println("RP2350. Init ready.");
}

void loop() {
  static uint32_t startTime = millis();

  //lv_timer_handler();
  //sleep_ms(5);


  if ((millis() - startTime) >= 1000UL)
  {
    startTime = millis();
    Serial.println("Looping");
  }

  lv_timer_handler_run_in_period(5);
}
