#include "Screenbase.h"
//#include "extras.h"
//#include "./src/AP33772SS/AP33772S.h"

// ── PDO type constants ────────────────────────────────────────────────────────
#define PDO_TYPE_FIXED       (0)
#define PDO_TYPE_PPS         (1)   // type=1 in SPR slot (index 1–7)
#define PDO_TYPE_AVS         (2)   // type=1 in EPR slot (index 8–13)

static lv_obj_t * pdo_list_cont = NULL;

lv_obj_t * screen3 = NULL;
lv_obj_t * getpdolistbutton = NULL;

#define CHARGECOLOR LV_PALETTE_GREEN
#define DISCHARGECOLOR LV_PALETTE_RED
#define INITCOLOR LV_PALETTE_GREY

#define MAX_PDO_ENTRIES      (13)

static lv_obj_t * PDOCells[MAX_PDO_ENTRIES] = {NULL};

lv_obj_t * CreatePDOCell(lv_obj_t * parent);
void Screen3SetPDO(
  uint8_t  index,          // 1-based (1–13)
  bool     valid,          // detect bit = 1
  bool     isEPR,          // true for index 8–13
  uint8_t  type,           // PDO_TYPE_FIXED / _PPS / _AVS
  uint16_t minVoltage_mV,  // 0 for Fixed; 3300 for PPS; 15000 for AVS
  uint16_t maxVoltage_mV,
  uint16_t maxCurrent_mA);  // Approximate upper bound of current range

static void btn_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * user = lv_event_get_user_data(e);  
  /*The original target of the event. Can be the buttons or the container*/
  lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);

  if ( (btn != NULL) && (code == LV_EVENT_CLICKED) )
  {
    if (btn == getpdolistbutton)
    {
      // Cleanup all PDO cells !!
      for(int i = 1; i <= MAX_PDO_ENTRIES; i++)
      {
        Screen3SetPDO(i,false,false,0,0,0,0);
      }
    }
    else
    {
      // Select PDO buttons only
      /*The current target is always the container as the event is added to it*/
      lv_obj_t * cont = lv_event_get_current_target(e);
      /*If container was clicked do nothing*/
      if (btn == cont) return;
    }

    lv_event_cb_t event_cb = GetEvent();
    event_cb(e);
  }
}

void Setup_Screen(lv_obj_t * cont)
{
  lv_obj_t * obj = NULL;
  lv_obj_t * label = NULL;
  lv_obj_t * btn = NULL;      
  lv_obj_t * cell = NULL;
  lv_obj_t * pdo_list_cont = NULL;

  if (lv_obj_get_child_count(cont) == 0)
  {
    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_width(&label_style, LV_PCT(100));
    lv_style_set_height(&label_style, LV_SIZE_CONTENT);
    lv_style_set_border_width(&label_style, 4);
    lv_style_set_border_side(&label_style, LV_BORDER_SIDE_FULL);
    lv_style_set_border_color(&label_style, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    //lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    //lv_style_set_radius(&label_style, 4); // <--- Sets the radius to 10 pixels    

    lv_style_set_text_font(&label_style, &lv_font_montserrat_24);
    lv_style_set_text_align(&label_style, LV_TEXT_ALIGN_CENTER);    

    /*
    label = lv_label_create(cont);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(label, "Click to select PDO");
    lv_obj_add_style(label , &label_style, 0);
    lv_obj_set_style_text_color(label,lv_palette_main(LV_PALETTE_INDIGO), 0);
    lv_obj_set_style_pad_bottom(label, 4, LV_PART_MAIN);    
    lv_obj_set_style_pad_top(label, 4, LV_PART_MAIN);    
    */

    btn = lv_button_create(cont);
    getpdolistbutton = btn;
    //lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, lv_pct(15));
    //lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);    
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_pad_top(btn, 14, LV_PART_MAIN | LV_STATE_DEFAULT);    
    lv_obj_set_style_pad_bottom(btn, 14, LV_PART_MAIN | LV_STATE_DEFAULT);    
    lv_obj_set_style_margin_top(btn, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_margin_bottom(btn, 14, LV_PART_MAIN | LV_STATE_DEFAULT);    
  
    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);  
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "Click to refresh PDOs");
    //lv_obj_add_style(label , &label_style, 0);
 
    //lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    //lv_label_set_text(label, "GO DISCHARGE");


    // PDO list

    pdo_list_cont = lv_obj_create(cont);
    lv_obj_set_size(pdo_list_cont, lv_pct(100), lv_pct(80));
    //lv_obj_align(discharge_cont, LV_ALIGN_TOP_LEFT, 0, 0);    
    lv_obj_align_to(pdo_list_cont,btn,LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  
    //lv_obj_set_style_pad_left(discharge_cont, 2, LV_PART_MAIN);  
    lv_obj_set_scrollbar_mode(pdo_list_cont, LV_SCROLLBAR_MODE_ON);  
    lv_obj_set_scroll_dir(pdo_list_cont,LV_DIR_VER);  

    lv_obj_add_event_cb(pdo_list_cont, btn_event_cb, LV_EVENT_CLICKED, screen3); 


    //lv_obj_set_style_pad_top(label, 2, LV_PART_MAIN);  

    obj = pdo_list_cont;

    for(int i = 0; i < MAX_PDO_ENTRIES; i++)
    {
      cell = CreatePDOCell(pdo_list_cont);
      if (i==0)
      {
        lv_obj_align(cell, LV_ALIGN_TOP_LEFT, -4, 0);    

      }
      else
      {
        lv_obj_align_to(cell,obj,LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);  
      }
      PDOCells[i] = cell;
      obj = cell;      
    }
  }
}

void Setup_Screen3(byte index)
{
  lv_obj_t * obj = NULL;

  obj = GetInfoObject();
  if (obj != NULL) lv_label_set_text(obj,"PDO-list");

  obj = GetButtonLabelObject();
  if (obj != NULL)
  {
    lv_label_set_text(obj,"---");
    // Get the navigation button itself
    //btn_next_state = (uint16_t)((uint8_t)SCREENINDEX << 8 | (uint8_t)btn_next);    
  }

  SetContentObject(screen3);
  if (screen3 != NULL) Setup_Screen(screen3);
}

lv_obj_t * CreatePDOCell(lv_obj_t * parent)
{
  lv_obj_t * label;
  lv_obj_t * chk;  
  lv_obj_t * ta;
  lv_obj_t * cell;
  lv_obj_t * obj;  
  lv_palette_t p = INITCOLOR;    

  static lv_style_t ta_style;
  lv_style_init(&ta_style);
  lv_style_set_text_font(&ta_style, &lv_font_montserrat_14);  /*Set a larger font*/ 
  lv_style_set_pad_all(&ta_style, 4);  
  lv_style_set_radius(&ta_style, 0);    

  cell = lv_obj_create(parent);

  lv_obj_add_flag(cell, LV_OBJ_FLAG_EVENT_BUBBLE);

  lv_obj_remove_style_all(cell);  
  lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);  
  lv_obj_set_size(cell, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);          
  lv_obj_set_style_pad_column(cell, 10, 0); //Space between widgets
  lv_obj_set_style_bg_color(cell, lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);
  lv_obj_set_style_border_color(cell, lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  lv_obj_set_style_border_width(cell, 2, LV_PART_MAIN);  

  lv_obj_set_style_pad_top(cell, 2, LV_PART_MAIN);  
  lv_obj_set_style_pad_bottom(cell, 2, LV_PART_MAIN);    

  lv_obj_set_style_shadow_width(cell, 5, 0);
  //lv_obj_set_style_shadow_ofs_x(cell, 5, 0);
  lv_obj_set_style_shadow_ofs_y(cell, 5, 0);  
  lv_obj_set_style_shadow_opa(cell, LV_OPA_50, 0);

  label = lv_label_create(cell);
  lv_obj_set_width(label, 40);
  lv_label_set_text(label, "-");
  lv_obj_add_style(label, &ta_style, 0);      

  label = lv_label_create(cell);
  lv_obj_set_width(label, 60);
  lv_label_set_text(label, "-");
  lv_obj_add_style(label, &ta_style, 0);      

  ta = lv_label_create(cell);
  lv_obj_add_style(ta, &ta_style, 0);      
  lv_obj_set_style_width(ta, 150, LV_PART_MAIN);
  lv_obj_set_style_border_width(ta, 2, LV_PART_MAIN);  
  lv_obj_set_style_border_color(ta, lv_palette_main(p), LV_PART_MAIN);
  lv_obj_set_style_text_color(ta, lv_palette_darken(p,4), LV_PART_SELECTED); 
  lv_obj_set_style_bg_color(ta, lv_palette_lighten(p,4), LV_PART_MAIN);
  lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_radius(ta, 0, LV_PART_MAIN);    
  lv_obj_set_style_pad_top(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_left(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_right(ta, 4, LV_PART_MAIN);

  lv_label_set_text(ta, "--- Volts");  

  ta = lv_label_create(cell);
  lv_obj_add_style(ta, &ta_style, 0);      
  lv_obj_set_style_width(ta, 90, LV_PART_MAIN);
  lv_obj_set_style_border_width(ta, 2, LV_PART_MAIN);  
  lv_obj_set_style_border_color(ta, lv_palette_main(p), LV_PART_MAIN);
  lv_obj_set_style_text_color(ta, lv_palette_darken(p,4), LV_PART_SELECTED); 
  lv_obj_set_style_bg_color(ta, lv_palette_lighten(p,4), LV_PART_MAIN);
  lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_radius(ta, 0, LV_PART_MAIN);    
  lv_obj_set_style_pad_top(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_left(ta, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_right(ta, 4, LV_PART_MAIN);

  lv_label_set_text(ta, "--- Amps");  

  lv_obj_t *cell_button = lv_btn_create(cell);

  lv_obj_add_flag(cell_button, LV_OBJ_FLAG_EVENT_BUBBLE);

  lv_obj_set_size(cell_button, LV_SIZE_CONTENT, lv_pct(90));
  lv_obj_add_flag(cell_button, LV_OBJ_FLAG_FLOATING);
  lv_obj_align(cell_button, LV_ALIGN_RIGHT_MID, 0, 0);  
  
  lv_obj_set_style_translate_x(cell_button, -5, LV_PART_MAIN| LV_STATE_DEFAULT);          
  lv_obj_set_style_pad_top(cell_button, 8, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(cell_button, 8, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cell_button, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cell_button, 6, LV_PART_MAIN);

  lv_obj_set_style_border_width(cell_button, 2, LV_PART_MAIN);  
  lv_obj_set_style_border_color(cell_button, lv_palette_main(p), LV_PART_MAIN);
  lv_obj_set_style_text_color(cell_button, lv_palette_darken(p,4), LV_PART_SELECTED); 
  lv_obj_set_style_bg_color(cell_button, lv_palette_lighten(p,4), LV_PART_MAIN);
  lv_obj_set_style_bg_color(cell_button,lv_palette_darken(p,4), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(cell_button,lv_palette_darken(p,100), LV_PART_MAIN | LV_STATE_DISABLED);

  obj = lv_label_create(cell_button);
  lv_label_set_text(obj, "Select");
  lv_obj_center(obj);
  
  return (cell);
}

void Screen3SetPDO(
  uint8_t  index,          // 1-based (1–13)
  bool     valid,          // detect bit = 1
  bool     isEPR,          // true for index 8–13
  uint8_t  type,           // PDO_TYPE_FIXED / _PPS / _AVS
  uint16_t minVoltage_mV,  // 0 for Fixed; 3300 for PPS; 15000 for AVS
  uint16_t maxVoltage_mV,
  uint16_t maxCurrent_mA)  // Approximate upper bound of current range
{
  if (index > 0) 
  {
    lv_obj_t * cell = PDOCells[index-1];

    if (cell != NULL) 
    {
      lv_obj_t * obj; 
      lv_palette_t p;  

      if (!valid)
      {
        if (!lv_obj_has_flag(cell, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN);
        p = INITCOLOR;    
        return;
      }
      else
      {
        if (lv_obj_has_flag(cell, LV_OBJ_FLAG_HIDDEN)) lv_obj_remove_flag(cell, LV_OBJ_FLAG_HIDDEN);    
        if (isEPR) p = DISCHARGECOLOR;
        if (!isEPR) p = CHARGECOLOR;  
      }

      // Get SPR/EPR label
      obj = lv_obj_get_child(cell, 0);  
      if (lv_obj_check_type(obj, &lv_label_class))
      {
        if (!isEPR) lv_label_set_text(obj, "SPR");
        if (isEPR) lv_label_set_text(obj, "EPR");
      }

      // Get type label
      obj = lv_obj_get_child(cell, 1);  
      if (lv_obj_check_type(obj, &lv_label_class))
      {
        if (type==PDO_TYPE_FIXED) lv_label_set_text(obj, "Fixed");
        if (type==PDO_TYPE_PPS) lv_label_set_text(obj, "PPS");
        if (type==PDO_TYPE_AVS) lv_label_set_text(obj, "AVS");
      }

      int cv;
      int whole;
      int hundredths;

      // Get voltage label
      obj = lv_obj_get_child(cell, 2);  
      if (lv_obj_check_type(obj, &lv_label_class))
      {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), LV_PART_MAIN);
        lv_obj_set_style_text_color(obj, lv_palette_darken(p,4), LV_PART_SELECTED); 
        lv_obj_set_style_bg_color(obj, lv_palette_lighten(p,4), LV_PART_MAIN);

        // Round to the nearest hundredth of a volt (centi-volt)
        cv = (maxVoltage_mV + 5) / 10;
        // Separate into whole volts and the remaining hundredths
        whole = cv / 100;
        hundredths = cv % 100;

        //if ((minVoltage_mV==0) && (type==0))
        if (type==0)
        {
          lv_label_set_text_fmt(obj, "%d.%02d Volts", whole, hundredths);  
        }
        else
        {
          // Round to the nearest hundredth of a volt (centi-volt)
          cv = (minVoltage_mV + 5) / 10;
          // Separate into whole volts and the remaining hundredths
          int minvolts = cv / 100;
          int minhundredths = cv % 100;
          lv_label_set_text_fmt(obj, "%d.%02d - %d.%02d Volts", minvolts, minhundredths, whole, hundredths);  
        }
      }
      // Get current label
      obj = lv_obj_get_child(cell, 3);  
      if (lv_obj_check_type(obj, &lv_label_class))
      {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), LV_PART_MAIN);
        lv_obj_set_style_text_color(obj, lv_palette_darken(p,4), LV_PART_SELECTED); 
        lv_obj_set_style_bg_color(obj, lv_palette_lighten(p,4), LV_PART_MAIN);

        // Round to the nearest hundredth of an amp (centi-amp)
        cv = (maxCurrent_mA + 5) / 10;
        // Separate into whole volts and the remaining hundredths
        whole = cv / 100;
        hundredths = cv % 100;
        lv_label_set_text_fmt(obj, "%d.%02d Amps", whole, hundredths);  
      }

      // Get select buton
      obj = lv_obj_get_child(cell, 4);  
      if (lv_obj_check_type(obj, &lv_button_class))
      {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), LV_PART_MAIN);
        lv_obj_set_style_text_color(obj, lv_palette_darken(p,4), LV_PART_SELECTED); 
        lv_obj_set_style_bg_color(obj, lv_palette_lighten(p,4), LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj,lv_palette_darken(p,4), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(obj,lv_palette_darken(p,100), LV_PART_MAIN | LV_STATE_DISABLED);

        cv = index;
        if (isEPR) cv += 7;

        //lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, (void *)((uint16_t)((uint8_t)SCREENINDEX << 8 | (uint8_t)index)));   
        //lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, (void *)((uint8_t)index)); 
        lv_obj_set_user_data(obj, (void *)((uint8_t)index));
      }
    }
  }
}
