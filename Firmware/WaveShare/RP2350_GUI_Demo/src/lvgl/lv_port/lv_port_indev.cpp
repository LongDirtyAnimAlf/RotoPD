#include "lv_port_indev.h"
#include "lvgl.h"
#include "./../../bsp/bsp_gt911.h"
#include "./../../bsp/bsp_i2c.h"


int16_t touch_last_x = 0, touch_last_y = 0;

//static bsp_touch_interface_t *touch_if = NULL;

bool touch_init(int16_t w, int16_t h, uint8_t r)
{

    // We need I2C, so init here.
    bsp_i2c_init();

    static bsp_touch_info_t touch_info;
    touch_info.width = w;
    touch_info.height = h;

    switch (r)
    {
        case 0: {touch_info.rotation = 2; break;}
        case 1: {touch_info.rotation = 3; break;}
        case 2: {touch_info.rotation = 0; break;}
        case 3: {touch_info.rotation = 1; break;}
    }

    bsp_touch_interface_t * touch_if = NULL;

    bsp_touch_new_gt911(&touch_if, &touch_info);

    if (touch_if != NULL)
    {
        touch_if->init();

        //touch_if->set_rotation(uint16_t rotation);
        //touch_if->get_rotation(uint16_t *rotation);

        return (true);
    }

    return (false);
}

/*Will be called by the library to read the touchpad*/
bool touch_touched(void)
{
    bsp_touch_data_t touch_data;

    bsp_touch_interface_t * touch_if = bsp_gt911_get_touch_interface();

    touch_if->read();
    
    /*Save the pressed coordinates and the state*/
    if (touch_if->get_data(&touch_data))
    {
        
      touch_last_x = touch_data.coords[0].x;
      touch_last_y = touch_data.coords[0].y;

      return (true);
    }
    return (false);   
}

bool touch_has_signal(void)
{
  bsp_touch_interface_t * touch_if = bsp_gt911_get_touch_interface();
  return (touch_if != NULL);
}

bool touch_released(void)
{
  return false;
}
