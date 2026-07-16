#include "Screenbase.h"

#define NUMBEROFDISPLAY 5

lv_obj_t * screen1 = NULL;

static lv_obj_t * ta1 = NULL;
static lv_obj_t * ta2 = NULL;

lv_obj_t * testdischargebutton = NULL;
lv_obj_t * startdischargebutton = NULL;
lv_obj_t * testchargebutton = NULL;
lv_obj_t * startchargebutton = NULL;

lv_obj_t * zerocapacitybutton = NULL;
lv_obj_t * zeroenergybutton = NULL;
lv_obj_t * zerotimebutton = NULL;

static bool chargedisabled = true;
static bool dischargedisabled = false;

static lv_obj_t * ThresholdLed[tmLast] = {NULL};

const char* const ThresholdNames[] = {
   "None",
   "Vmax",
   "Vmin",
   "-dV",
   "pV",
   "Tmax",
   "dT",
   "dVT",
   "time",
   "Cmax"
   };

static lv_obj_t * Display[NUMBEROFDISPLAY] = {NULL};

static void ChargeStatus(bool Status)
{
  if (Status)
  {
    lv_obj_add_state(testchargebutton,LV_STATE_DISABLED);
    lv_obj_add_state(startchargebutton,LV_STATE_DISABLED);
    lv_obj_add_state(ta2,LV_STATE_DISABLED);          
    lv_obj_clear_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  }
  else
  {
    lv_obj_remove_state(testchargebutton,LV_STATE_DISABLED);
    lv_obj_remove_state(startchargebutton,LV_STATE_DISABLED);
    lv_obj_remove_state(ta2,LV_STATE_DISABLED);          
    lv_obj_add_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  }
  lv_obj_remove_state(testchargebutton,LV_STATE_CHECKED);
  lv_obj_remove_state(startchargebutton,LV_STATE_CHECKED);  
}

static void DischargeStatus(bool Status)
{
  if (Status)
  {
    lv_obj_add_state(testdischargebutton,LV_STATE_DISABLED);
    lv_obj_add_state(startdischargebutton,LV_STATE_DISABLED);
    lv_obj_add_state(ta1,LV_STATE_DISABLED);
    lv_obj_clear_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);          
  }
  else
  {
    lv_obj_remove_state(testdischargebutton,LV_STATE_DISABLED);
    lv_obj_remove_state(startdischargebutton,LV_STATE_DISABLED);
    lv_obj_remove_state(ta1,LV_STATE_DISABLED);
    lv_obj_add_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);          
  }
  lv_obj_remove_state(testdischargebutton,LV_STATE_CHECKED);
  lv_obj_remove_state(startdischargebutton,LV_STATE_CHECKED);  
}

static void setbuttons(lv_obj_t * btn, bool checked)
{
  static lv_obj_t ** btns[4] = {&testdischargebutton,&startdischargebutton,&testchargebutton,&startchargebutton};
  lv_obj_t * obj;  


  if (checked)  
  {
    lv_obj_add_state(btn,LV_STATE_CHECKED);

    for (byte i = 0; i < 4; i++)
    {
      obj = *btns[i];
     
      if ( (obj != NULL) && (obj != btn) )
      {
        lv_obj_add_state(obj,LV_STATE_DISABLED);

        lv_obj_add_state(zerocapacitybutton,LV_STATE_DISABLED);
        lv_obj_add_state(zeroenergybutton,LV_STATE_DISABLED);
        if (zerotimebutton != NULL) lv_obj_add_state(zerotimebutton,LV_STATE_DISABLED);        
        lv_obj_add_state(ta1,LV_STATE_DISABLED);
        lv_obj_add_state(ta2,LV_STATE_DISABLED);          
        lv_obj_clear_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);          
        lv_obj_clear_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);                    
      }
    }
  }
  else
  {
    ChargeStatus(chargedisabled);
    DischargeStatus(dischargedisabled);
    lv_obj_remove_state(zerocapacitybutton,LV_STATE_DISABLED);
    lv_obj_remove_state(zeroenergybutton,LV_STATE_DISABLED);
    if (zerotimebutton != NULL) lv_obj_remove_state(zerotimebutton,LV_STATE_DISABLED);            
  }
}

static void btn_event_cb(lv_event_t * e)
{
  bool checked = false;
  static bool longpress = false;

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * btn = lv_event_get_current_target_obj(e);
  lv_obj_t * user = lv_event_get_user_data(e);  

  if (btn != NULL)
  {
    if (code == LV_EVENT_CLICKED)
    {
      // Transport the event itself to the main application !!
      lv_event_cb_t event_cb = GetEvent();
      event_cb(e);
    }

    if ((!longpress) && (code == LV_EVENT_LONG_PRESSED))
    {
      longpress = true;
      checked = (lv_obj_get_state(btn) & LV_STATE_CHECKED);
      //if (checked) lv_obj_remove_state(btn,LV_STATE_CHECKED) ;else lv_obj_add_state(btn,LV_STATE_CHECKED);      
      setbuttons(btn, (!checked));      
      lv_obj_send_event(btn, LV_EVENT_VALUE_CHANGED, user);      
    }

    if ((longpress) && (code == LV_EVENT_VALUE_CHANGED))
    {
      // Transport the event itself to the main application !!
      lv_event_cb_t event_cb = GetEvent();
      event_cb(e);
    }

    if (code == LV_EVENT_RELEASED)
    {
      longpress = false;    
    }

  }
}

void keyboard_event_cb(lv_event_t * e)
{
  //lv_keyboard_def_event_cb(kb, event);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY)
  {
    if (code == LV_EVENT_READY)
    {
      //Serial.println("Keyboard ready !!!!");
      //String ta_text = lv_textarea_get_text(lv_keyboard_get_textarea(event_object));
      //Serial.println(ta_text);      

    }

  }
  else
  if (code == LV_EVENT_CANCEL)
  {
  }

  //lv_textarea_get_text(lv_keyboard_get_textarea(kb));

  lv_event_cb_t event_cb = GetEvent();
  event_cb(e);
}

static void ta_event_cb_local(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * label = lv_event_get_target(e);
  
  if (code == LV_EVENT_CLICKED)
  {
    //NumpadShowText(lv_label_get_text(label));
    NumpadShowLabel(label);    
    //lv_event_cb_t event_cb = GetEvent();
    //event_cb(e);
  }
}

void Setup_Screen1(byte index)
{
  lv_obj_t * obj = NULL;
  lv_obj_t * label = NULL;
  lv_obj_t * cell = NULL;
  lv_obj_t * grid = NULL;
  lv_obj_t * cont = NULL;
  lv_obj_t * ta = NULL;  
  lv_obj_t * btn = NULL;    

  obj = GetInfoObject();
  if (obj != NULL) lv_label_set_text(obj,"DATA");

  obj = GetButtonLabelObject();
  if (obj != NULL) lv_label_set_text(obj,"Chart");

  SetContentObject(screen1);
  cont = screen1;

  if ((cont != NULL) && (lv_obj_get_child_count(cont) == 0))
  {
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_24);  /*Set a larger font*/ 
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);  
    lv_style_set_border_width(&style, 4);
    lv_style_set_pad_all(&style, 4);  
    
    // Create container for top
    lv_obj_t * top_cont = lv_obj_create(cont);
    lv_obj_remove_style_all(top_cont);
    lv_obj_set_size(top_cont, lv_pct(100), lv_pct(20));
    lv_obj_align(top_cont, LV_ALIGN_TOP_MID, 0, 0);    
    
    // Create container for discharge setting and test  
    obj = lv_obj_create(top_cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, lv_pct(50), lv_pct(100));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);    
  
    //Create the label
    label = lv_label_create(obj);
    lv_obj_set_width(label, lv_pct(100));    
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_label_set_text(label, "Discharge current [mA]");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    ta = lv_label_create(obj);
    lv_obj_add_style(ta, &input_label_style, 0);
    lv_label_set_text(ta, "0");
    lv_obj_align_to(ta, label, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 10); 
    lv_obj_add_flag(ta, LV_OBJ_FLAG_CLICKABLE);          
    lv_obj_add_event_cb(ta, ta_event_cb_local, LV_EVENT_CLICKED, NULL);
  
    ta1 = ta;
  
    //Create the test button
    btn = lv_button_create(obj);
    lv_obj_set_size(btn, lv_pct(40), LV_SIZE_CONTENT);
    lv_obj_align_to(btn,label,LV_ALIGN_OUT_BOTTOM_RIGHT, -20, 10);    
  
    //lv_obj_set_style_bg_color(btn,lv_palette_darken(LV_PALETTE_INDIGO,3), LV_PART_MAIN); 
    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_CHECKED);       
    //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);

    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);   


    //lv_group_remove_obj(btn);    
  
    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "TEST");
  
    testdischargebutton = btn;

    lv_obj_set_user_data(ta1, testdischargebutton);
  
    // Create container for charge setting and test  
    obj = lv_obj_create(top_cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, lv_pct(50), lv_pct(100));
    lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, 0, 0);    
  
    //Create the label
    label = lv_label_create(obj);
    lv_obj_set_width(label, lv_pct(100));    
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_label_set_text(label, "Charge current [mA]");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  
    //Create the current setting text area
    ta = lv_label_create(obj);
    lv_obj_add_style(ta, &input_label_style, 0);
    lv_label_set_text(ta, "0");
    lv_obj_align_to(ta, label, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 10); 
    lv_obj_add_flag(ta, LV_OBJ_FLAG_CLICKABLE);          
    lv_obj_add_event_cb(ta, ta_event_cb_local, LV_EVENT_CLICKED, NULL);
  
    ta2 = ta;
  
    //Create the test button
    btn = lv_button_create(obj);
    lv_obj_set_size(btn, lv_pct(40), LV_SIZE_CONTENT);
    lv_obj_align_to(btn,label,LV_ALIGN_OUT_BOTTOM_RIGHT, -20, 10);    
  
    //lv_obj_set_style_bg_color(btn,lv_palette_darken(LV_PALETTE_INDIGO,3), LV_PART_MAIN); 
    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_CHECKED);       
    //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, (void *)btn_test_charge);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);   
  
    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "TEST");
  
    testchargebutton = btn;
  
    lv_obj_set_user_data(ta2, testchargebutton);

    static lv_coord_t col_dsc2[] = { LV_GRID_FR(7), LV_GRID_FR(14), LV_GRID_FR(4), LV_GRID_FR(6), LV_GRID_TEMPLATE_LAST };
    //static lv_coord_t row_dsc2[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row_dsc2[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
  
    grid = lv_obj_create(cont);
  
    lv_obj_set_size(grid, lv_pct(95), 54*NUMBEROFDISPLAY);
    //lv_obj_set_size(grid,LV_SIZE_CONTENT,LV_SIZE_CONTENT);
    //lv_obj_set_size(grid,lv_pct(80),lv_pct(100));
    //lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 0);
  
    lv_obj_set_style_pad_top(grid, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(grid, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_left(grid, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_right(grid, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(grid, 4, 0);
    lv_obj_set_style_pad_column(grid, 4, 0);
    lv_obj_set_grid_dsc_array(grid, col_dsc2, row_dsc2);
  
    int i,row,col;
  
    for(i = 0; i < NUMBEROFDISPLAY; i++)
    {
  
      cell = lv_obj_create(grid);
  
      if (cell != NULL)
      {
        row = i;
        col = 1;
  
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
  
        lv_obj_set_style_bg_color(cell,lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  
        lv_obj_set_style_pad_all(cell, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(cell, 10, LV_PART_MAIN);
  
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
  
        lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);          
        //lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);          
        lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);          
        lv_obj_set_style_pad_column(cell, 10, 0); //Space between widgets
  
        byte count = 5;
        //if (row==0) count = 6;        
        if ((row==2) || (row==3)) count = 6;
        if (row==4) count = 7;
        obj = create_display(cell, lv_palette_main(LV_PALETTE_RED), false, count);
        Display[i] = obj;
  
        obj = lv_label_create(cell);
        lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT );	
        switch (row)
        {
          case 0: {lv_label_set_text(obj, "Volt"); break;}
          case 1: {lv_label_set_text(obj, "Amps"); break;}        
          case 2: {lv_label_set_text(obj, "W"); break;}                
          case 3: {lv_label_set_text(obj, "Wh"); break;}                        
          case 4: {lv_label_set_text(obj, "s"); break;}                        
        }
        lv_obj_set_style_translate_x(obj, -8, LV_PART_MAIN| LV_STATE_DEFAULT);          
        lv_obj_set_style_translate_y(obj, -6, LV_PART_MAIN| LV_STATE_DEFAULT);
      }
  
      cell = lv_obj_create(grid);
  
      if (cell != NULL)
      {
        row = i;
        col = 0;
  
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);      
  
        lv_obj_set_style_bg_color(cell,lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  
        lv_obj_set_style_pad_all(cell, 0, LV_PART_MAIN);
        //lv_obj_set_style_pad_left(cell, 10, LV_PART_MAIN);
  
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
  
        //lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);          
        //lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);          
        //lv_obj_set_style_pad_column(cell, 10, 0); //Space between widgets
  
        obj = lv_label_create(cell);
        lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(obj, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);        
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);	
        switch (row)
        {
          case 0: {lv_label_set_text(obj, "Voltage"); break;}
          case 1: {lv_label_set_text(obj, "Current"); break;}        
          case 2: {lv_label_set_text(obj, "Power"); break;}                
          case 3: {lv_label_set_text(obj, "Energy"); break;}                        
          case 4: {lv_label_set_text(obj, "Time"); break;}                        
        }
      }
  
      cell = lv_obj_create(grid);
  
      if (cell != NULL)
      {
        row = i;
        col = 2;
  
        lv_obj_remove_style_all(cell);
  
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);      
  
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
  
        //lv_obj_set_style_bg_color(cell,lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  
        lv_obj_set_style_pad_all(cell, 0, LV_PART_MAIN);
        //lv_obj_set_style_pad_left(cell, 10, LV_PART_MAIN);
  
        obj = lv_button_create(cell);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(80));
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        //lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
  
        if ((row==2) || (row==3) || (row==4))
        {
          //lv_obj_add_event_cb(obj, event_cb, LV_EVENT_VALUE_CHANGED, (void *)btn_start_discharge);
          lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_RED,4), LV_PART_MAIN);      
  
          label = lv_label_create(obj);
          lv_obj_set_style_text_font(label, &lv_font_montserrat_12, LV_PART_MAIN| LV_STATE_DEFAULT);  
          lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
          lv_label_set_text(label, "Zero");
  
          if (row==2)
          {
            lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);
            zerocapacitybutton = obj;
          }
          if (row==3)
          {
            lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);
            zeroenergybutton = obj;
          }
          if (row==4)
          {
            lv_label_set_text(label, "Auto");
            //lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);
            zerotimebutton = obj;
            lv_obj_add_state(obj,LV_STATE_DISABLED);
          }
        }
        else
        {
          lv_obj_add_state(obj,LV_STATE_DISABLED);
        }
      }  
    }  
  
    cell = lv_obj_create(grid);
    if (cell != NULL)
    {
      lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, NUMBEROFDISPLAY);
      //lv_obj_remove_style_all(cell);
      lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);      
      lv_obj_set_style_pad_all(cell, 5, LV_PART_MAIN);
      lv_obj_set_style_pad_top(cell, 10, LV_PART_MAIN);
      lv_obj_set_style_bg_color(cell,lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
      lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);          
      lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);          
      lv_obj_set_style_pad_row(cell, 14, 0); //Space between widgets

      // Create threshold LEDs.
      for(i = (tmNONE+1); i < tmLast; i++)
      {
        if ((i==tmDVT) || (i==tmDELTAT)) continue;
        obj  = lv_led_create(cell);
        ThresholdLed[i]=obj;
        lv_led_set_brightness(obj, 255);
        lv_led_set_color(obj, lv_palette_main(LV_PALETTE_ORANGE));
        lv_led_off(obj);
        lv_obj_set_size(obj, lv_pct(100),LV_SIZE_CONTENT);  
        obj = lv_label_create(obj);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN| LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT );	
        lv_label_set_text(obj, ThresholdNames[i]);
      }
    }

    lv_obj_t * bottom_cont = lv_obj_create(cont);
    lv_obj_remove_style_all(bottom_cont);
    lv_obj_clear_flag(bottom_cont, LV_OBJ_FLAG_SCROLLABLE);  
    lv_obj_set_size(bottom_cont, lv_pct(100), lv_pct(15));
    lv_obj_align(bottom_cont, LV_ALIGN_BOTTOM_MID, 0, 0);    
    //lv_obj_set_flex_align(bottom_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);          
  
    btn = lv_button_create(bottom_cont);
    lv_obj_set_size(btn, lv_pct(48), LV_SIZE_CONTENT);
    //lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);    
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 6, 0);
    //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, (void *)btn_start_discharge);      
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);   

    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_CHECKED); 
  
    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "GO DISCHARGE");
  
    startdischargebutton = btn;
  
    btn = lv_button_create(bottom_cont);
    lv_obj_set_size(btn, lv_pct(48), LV_SIZE_CONTENT);
    //lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);    
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -6, 0);
    //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    //lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, (void *)btn_start_charge);      
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);   

    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_CHECKED);
  
    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);  
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "GO CHARGE");
  
    startchargebutton = btn;
  }
}

void Screen1SetThresholdLedEnabled(TThresholdModes Mode, bool On)
{
  lv_obj_t * obj = ThresholdLed[Mode];
  if (obj != NULL)
  {
    obj = lv_obj_get_child(obj, 0);  
    if (On)
    {
      lv_obj_set_style_text_color(obj, lv_palette_darken(LV_PALETTE_YELLOW,1), LV_PART_MAIN | LV_STATE_DEFAULT );
    }
    else
    {
      lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT );
    }
  }
}

void Screen1SetThresholdLed(TThresholdModes Mode, bool On)
{
  lv_obj_t * obj = ThresholdLed[Mode];
  if (obj != NULL)
  {
    if (On) lv_led_on(obj); else lv_led_off(obj);
  }
}

void Screen1AddVData(word V)
{
  if (Display[0] != NULL) SetDisplaymV(Display[0], V);  
}

void Screen1AddIData(word I)
{
  if (Display[1] != NULL) SetDisplaymV(Display[1], I);  
}

void Screen1AddVIData(word V, word I)
{
  Screen1AddVData(V);
  Screen1AddIData(I);  
}

void Screen1AddTData(dword T)
{
  if (Display[4] != NULL) SetDisplayV(Display[4], T);
}

void Screen1AddEData(dword E)
{
  if (Display[3] != NULL) SetDisplaymV(Display[3], E);
}

void Screen1AddPData(dword P)
{
  if (Display[2] != NULL) SetDisplaymV(Display[2], P);
}

void Screen1AddEPData(dword E, dword P)
{
  Screen1AddEData(E);
  Screen1AddPData(P);  
}

void Screen1SetData(PBatterySetting SET)
{
  Settings = SET;

  #ifndef STANDALONE

  lv_obj_add_state(ta1,LV_STATE_DISABLED);
  lv_obj_add_state(ta2,LV_STATE_DISABLED);
  
  lv_obj_add_state(testdischargebutton,LV_STATE_DISABLED);
  lv_obj_add_state(startdischargebutton,LV_STATE_DISABLED);
  lv_obj_add_state(testchargebutton,LV_STATE_DISABLED);
  lv_obj_add_state(startchargebutton,LV_STATE_DISABLED);
  
  lv_obj_add_state(zerocapacitybutton,LV_STATE_DISABLED);
  lv_obj_add_state(zeroenergybutton,LV_STATE_DISABLED);
  if (zerotimebutton != NULL) lv_obj_add_state(zerotimebutton,LV_STATE_DISABLED);

  #else

  chargedisabled = (SET->Stages[FIXEDCHARGESTAGENUMBER].Status == smDisabled);
  dischargedisabled = (SET->Stages[FIXEDDISCHARGESTAGENUMBER].Status == smDisabled);
  
  ChargeStatus(chargedisabled);
  DischargeStatus(dischargedisabled);

  PRunDatas RDS = &SET->TestData.RunDatas;      
  Screen1AddVIData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);
  Screen1AddEPData((RDS->Energy / 1000),(RDS->LastBatteryData.P));
  Screen1AddTData(RDS->Time);

  //if (SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[tmMINV].Enabled)
  if (SET->TestData.Active == bmActive)
  {
    if (SET->TestData.SetStageMode == smCurrent) setbuttons(startdischargebutton, true);
    if (SET->TestData.SetStageMode == smCharge) setbuttons(startchargebutton, true);
    //setbuttons(startdischargebutton, (SET->TestData.SetStageMode == smCurrent));
    //setbuttons(startchargebutton, (SET->TestData.SetStageMode == smCharge));
  }
  else
  {
    if (SET->TestData.SetStageMode == smCurrent) setbuttons(testdischargebutton, true);
    if (SET->TestData.SetStageMode == smCharge) setbuttons(testchargebutton, true);
    //setbuttons(testdischargebutton, (SET->TestData.SetStageMode == smCurrent));
    //setbuttons(testchargebutton, (SET->TestData.SetStageMode == smCharge));
  }

  #endif STANDALONE

  // Set values
  char str[30] = "";
  dword Value;

  Value = 0;
  #ifdef STANDALONE
  Value = SET->Stages[FIXEDDISCHARGESTAGENUMBER].SetValue;
  #else
  if (SET->TestData.SetStageMode == smCurrent) Value = SET->TestData.SetStageValue;  
  #endif
  sprintf(str, "%d", Value);
  lv_label_set_text(ta1, str);

  Value = 0;
  #ifdef STANDALONE
  Value = SET->Stages[FIXEDCHARGESTAGENUMBER].SetValue;
  #else
  if (SET->TestData.SetStageMode == smCharge) Value = SET->TestData.SetStageValue;  
  #endif
  sprintf(str, "%d", Value);
  lv_label_set_text(ta2, str);  

  bool Enabled;

  for(byte i = (tmNONE+1); i < tmLast; i++)
  {
    Screen1SetThresholdLed((TThresholdModes)i, SET->TestData.RunDatas.ThresholdResult[i].Triggered);    

    #ifdef STANDALONE
    Enabled = false;
    Enabled |= ((SET->TestData.SetStageMode == smCurrent) && (SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[i].Enabled));
    Enabled |= ((SET->TestData.SetStageMode == smCharge) && (SET->Stages[FIXEDCHARGESTAGENUMBER].ThresholdSettings[i].Enabled));
    Screen1SetThresholdLedEnabled((TThresholdModes)i, Enabled);    
    #endif
  }
}
