#include "Screenbase.h"
#include "Screen1.h"
#include "Screen2.h"
#include "Screen3.h"

lv_obj_t * screenbase = NULL;
lv_obj_t * backbutton = NULL;
lv_obj_t * morebutton = NULL;

lv_style_t input_label_style;

static lv_obj_t * keyboardbase = NULL;
static lv_obj_t * newkeyboardbase = NULL;
static lv_event_cb_t event;
static lv_anim_t a;

PBatterySetting Settings = NULL;

void BaseScreenSetup(lv_obj_t * basescreen, lv_event_cb_t event_cb_more)
{
  if (basescreen == NULL) return;

  event = event_cb_more;

  lv_obj_t * nav = lv_obj_create(basescreen);
  lv_obj_remove_style_all(nav);
  lv_obj_clear_flag(nav, LV_OBJ_FLAG_SCROLLABLE);    
  lv_obj_set_style_pad_top(nav, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(nav, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_left(nav, 2, LV_PART_MAIN);
  lv_obj_set_style_pad_right(nav, 2, LV_PART_MAIN);
  lv_obj_set_size(nav, lv_pct(100), lv_pct(10));
  lv_obj_align(nav, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_obj_t * obj = NULL;

  obj = lv_button_create(nav);
  backbutton = obj;
  lv_obj_align(obj, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_size(obj, lv_pct(25), LV_SIZE_CONTENT);
  lv_obj_add_event_cb(obj, event_cb_more, LV_EVENT_CLICKED, NULL);
  lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);      

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "Back");

  obj = lv_button_create(nav);
  morebutton = obj;
  lv_obj_align(obj, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_size(obj, lv_pct(25), LV_SIZE_CONTENT);
  lv_obj_add_event_cb(obj, event_cb_more, LV_EVENT_CLICKED, NULL); 
  //lv_obj_add_event_cb(obj, event_cb_more, LV_EVENT_CLICKED, &btn_next_state); 
  lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);      

  obj = lv_label_create(obj);
  lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(obj, "More");

  lv_obj_t * basescreeninfo = lv_label_create(nav);
  lv_obj_align(basescreeninfo, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(basescreeninfo, &lv_font_montserrat_32, LV_PART_MAIN| LV_STATE_DEFAULT);  
  lv_label_set_text(basescreeninfo, "INFO");

  screen1 = lv_obj_create(basescreen);
  obj = screen1;
  lv_obj_remove_style_all(obj);
  // Add flag, indicating its a screen !!
  lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);  
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);    
  lv_obj_set_size(obj, lv_pct(100), lv_pct(90));
  lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
  //lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_GREEN,4), LV_PART_MAIN);      

  screen2 = lv_obj_create(basescreen);
  obj = screen2;
  lv_obj_remove_style_all(obj);
  // Add flag, indicating its a screen !!
  lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);  
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);    
  lv_obj_set_size(obj, lv_pct(100), lv_pct(90));
  lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
  //lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_RED,4), LV_PART_MAIN);      

  screen3 = lv_obj_create(basescreen);
  obj = screen3;
  lv_obj_remove_style_all(obj);
  // Add flag, indicating its a screen !!
  lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_1);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);  
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);    
  lv_obj_set_size(obj, lv_pct(100), lv_pct(90));
  lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
  //lv_obj_set_style_bg_color(obj,lv_palette_darken(LV_PALETTE_BLUE,4), LV_PART_MAIN);      
  
  lv_obj_move_to_index(nav, 0);
  lv_obj_move_to_index(screen1, 1);
  lv_obj_move_to_index(screen2, 2);
  lv_obj_move_to_index(screen3, 3);    

  lv_obj_move_to_index(morebutton, 0);
  lv_obj_move_to_index(basescreeninfo, 1);

  lv_style_init(&input_label_style);
  lv_style_init(&input_label_style);
  lv_style_set_width(&input_label_style, lv_pct(40));
  //lv_style_set_height(&input_label_style, LV_SIZE_CONTENT);
  lv_style_set_border_side(&input_label_style, LV_BORDER_SIDE_FULL);    
  lv_style_set_border_color(&input_label_style, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_text_color(&input_label_style, lv_color_black());
  lv_style_set_text_font(&input_label_style, &lv_font_montserrat_24);    
  lv_style_set_text_align(&input_label_style, LV_TEXT_ALIGN_CENTER);  
  lv_style_set_border_width(&input_label_style, 4);
  lv_style_set_pad_all(&input_label_style, 4);  
}

void newkeyboard_event_cb(lv_event_t * e)
{
  //lv_keyboard_def_event_cb(kb, event);
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY)
  {
    NumpadHide();
    lv_event_cb_t event_cb = GetEvent();
    event_cb(e);
  }
}

void NewKeyboardSetup(lv_obj_t * screen)
{
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_text_font(&style, &lv_font_montserrat_32);  /*Set a larger font*/ 
  lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);  
  lv_style_set_border_width(&style, 4);
  lv_style_set_pad_all(&style, 4);  

  static lv_style_t modal_style;
  lv_style_init(&modal_style);
  lv_style_set_bg_color(&modal_style, lv_palette_darken(LV_PALETTE_BLUE_GREY,4));
  lv_style_set_bg_opa(&modal_style, LV_OPA_50);
  lv_style_set_radius(&modal_style, 0); 
  lv_style_set_pad_all(&modal_style, 0);     

  /* Create a base object for the modal background */
  lv_obj_t* newkeyboardbase = lv_obj_create(screen);
  //lv_obj_remove_style_all(newkeyboardbase);
  lv_obj_set_size(newkeyboardbase, lv_pct(100), lv_pct(100));
  lv_obj_add_flag(newkeyboardbase, LV_OBJ_FLAG_HIDDEN);
  //lv_obj_set_style_bg_opa(newkeyboardbase, LV_OPA_TRANSP, 0);
  //lv_obj_set_style_bg_color(newkeyboardbase, lv_palette_darken(LV_PALETTE_RED,4), LV_PART_MAIN);    
  lv_obj_clear_flag(newkeyboardbase, LV_OBJ_FLAG_SCROLLABLE);    
  lv_obj_add_style(newkeyboardbase, &modal_style, LV_PART_MAIN);
  lv_obj_set_pos(newkeyboardbase, 0, 0);
  //lv_obj_set_size(newkeyboardbase, LV_HOR_RES, LV_VER_RES);
  //lv_obj_set_opa_scale_enable(newkeyboardbase, true); /* Enable opacity scaling for the animation */

  /*Create a text area. The keyboard will write here*/
  lv_obj_t* ta = lv_textarea_create(newkeyboardbase);
  lv_obj_add_style(ta, &style, 0);  
  lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 40);  
  lv_obj_set_height(ta, 75);
  //lv_obj_set_pos(ta, 10, 20);
  lv_obj_set_width(ta, lv_pct(40));
  lv_textarea_set_accepted_chars(ta, "0123456789");
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_text(ta, "0123");

  lv_obj_t* kb = lv_keyboard_create(newkeyboardbase); 
  lv_obj_align(kb, LV_ALIGN_CENTER, 0, 40); 
  
  lv_obj_add_event_cb(kb, newkeyboard_event_cb, LV_EVENT_ALL, ta);  
 
  static const char * newkb_map[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL };
  static const lv_btnmatrix_ctrl_t newkb_ctrl[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, newkb_map, newkb_ctrl);
  lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);  
  lv_obj_set_size(kb, LV_PCT(70), LV_PCT(70));
  lv_obj_set_style_bg_color(kb, lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);
  lv_obj_set_style_border_color(kb, lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  lv_keyboard_set_textarea(kb, ta);

  static lv_style_t newstyle;
  lv_style_init(&newstyle);
  lv_style_set_text_font(&newstyle, &lv_font_montserrat_24);  /*Set a larger font*/ 
  lv_style_set_text_align(&newstyle, LV_TEXT_ALIGN_CENTER);  
  lv_style_set_border_width(&newstyle, 4);
  lv_style_set_pad_all(&newstyle, 4);  
  lv_style_set_radius(&newstyle, 4);  
    
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_border_width(&style_btn, 4);
  lv_style_set_border_opa(&style_btn, LV_OPA_50);
  lv_style_set_border_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_border_side(&style_btn, LV_BORDER_SIDE_INTERNAL);
  lv_style_set_radius(&style_btn, 10);  
  lv_style_set_text_font(&style_btn, &lv_font_montserrat_24);  /*Set a larger font*/ 
  lv_style_set_text_align(&style_btn, LV_TEXT_ALIGN_CENTER);  

  lv_obj_add_style(kb, &newstyle, LV_PART_MAIN );
  lv_obj_add_style(kb, &style_btn, LV_PART_ITEMS);  

  lv_obj_set_style_margin_top(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_bottom(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_left(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_right(kb, 20, LV_PART_MAIN);

  /* Fade the message box in with an animation */
  lv_anim_init(&a);
  lv_anim_set_var(&a, newkeyboardbase);  
  lv_anim_set_duration(&a, 500);
  //lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
  //lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)fade_in_cb);  
  lv_anim_set_user_data(&a, newkeyboardbase);
  //lv_anim_start(&a);
}

void BaseKeyboardSetup(lv_obj_t * kb)
{
  if (kb == NULL) return;

  static const char * kb_map[] = {"1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, NULL };
  static const lv_btnmatrix_ctrl_t kb_ctrl[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
  lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_USER_1);  
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  //lv_obj_set_size(kb, LV_PCT(100), LV_PCT(100));
  lv_obj_set_size(kb, 250, 250);
  lv_obj_align(kb, LV_ALIGN_CENTER, 0, 0);  
  lv_obj_set_style_bg_color(kb, lv_palette_darken(LV_PALETTE_INDIGO,4), LV_PART_MAIN);
  lv_obj_set_style_border_color(kb, lv_palette_darken(LV_PALETTE_BLUE_GREY,4), LV_PART_MAIN);
  lv_keyboard_set_textarea(kb, NULL);

  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_text_font(&style, &lv_font_montserrat_24);  /*Set a larger font*/ 
  lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER);  
  lv_style_set_border_width(&style, 4);
  lv_style_set_pad_all(&style, 4);  
  lv_style_set_radius(&style, 4);  
  //lv_style_set_margin_top(&style, 20);
  //lv_style_set_margin_bottom(&style, 20);
  //lv_style_set_margin_left(&style, 20);
  //lv_style_set_margin_right(&style, 20);
    
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_border_width(&style_btn, 4);
  lv_style_set_border_opa(&style_btn, LV_OPA_50);
  lv_style_set_border_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_border_side(&style_btn, LV_BORDER_SIDE_INTERNAL);
  lv_style_set_radius(&style_btn, 10);  
  lv_style_set_text_font(&style_btn, &lv_font_montserrat_24);  /*Set a larger font*/ 
  lv_style_set_text_align(&style_btn, LV_TEXT_ALIGN_CENTER);  
  //lv_style_set_pad_all(&style_btn, 10);
  //lv_style_set_pad_gap(&style_btn, 5);

  lv_obj_add_style(kb, &style, LV_PART_MAIN );
  lv_obj_add_style(kb, &style_btn, LV_PART_ITEMS);  

  lv_obj_set_style_margin_top(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_bottom(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_left(kb, 20, LV_PART_MAIN);
  lv_obj_set_style_margin_right(kb, 20, LV_PART_MAIN);
}

void CreateBaseScreen(lv_event_cb_t event_cb_more)
{
  screenbase = lv_obj_create(NULL);
  keyboardbase = lv_keyboard_create(screenbase); 
  BaseScreenSetup(screenbase, event_cb_more);
  BaseKeyboardSetup(keyboardbase);
  NewKeyboardSetup(screenbase);  
}

void recursive_hide(lv_obj_t *parent, bool hide)
{
    uint32_t child_count = lv_obj_get_child_count(parent);
    lv_obj_t *child;

    for(uint32_t i=0;i<child_count;i++)
    {
      child = lv_obj_get_child(parent, i);
      if (hide)
      {
        if (!lv_obj_has_flag(child, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
      }
      else
      {
        if (lv_obj_has_flag(child, LV_OBJ_FLAG_HIDDEN)) lv_obj_remove_flag(child, LV_OBJ_FLAG_HIDDEN);    
      }
      recursive_hide(child,hide);
    }
}

void SetContentObject(lv_obj_t * content)
{
  lv_obj_t * obj;

  if (screenbase == NULL) return;

  // Get the navigation button
  obj = GetButtonLabelObject();
  obj = lv_obj_get_parent(obj);
  lv_obj_remove_state(obj,LV_STATE_DISABLED);

  byte Count = lv_obj_get_child_count(screenbase);
  if (Count)
  {
    do
    {
      Count--;
      obj = lv_obj_get_child(screenbase, Count);
      if ((obj != NULL) && (obj != content))
      {
        // If an child has this flag, its a screen !!
        if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1))
        {
          if (!lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
      }
    }
    while (Count);
  }

  if (content != NULL)
  {
    if (lv_obj_has_flag(content, LV_OBJ_FLAG_HIDDEN)) lv_obj_remove_flag(content, LV_OBJ_FLAG_HIDDEN);
  }
}

lv_obj_t * GetContentObject(byte contentindexnumber)
{
  lv_obj_t * obj = NULL;

  if (screenbase == NULL) return (NULL);

  // Get the navigation button
  obj = GetButtonLabelObject();
  obj = lv_obj_get_parent(obj);
  lv_obj_remove_state(obj,LV_STATE_DISABLED);

  for(byte i = 1; i < 4; i++)  
  {
    obj = lv_obj_get_child(screenbase, i);
    if ((obj != NULL) && (contentindexnumber != i))
    //if (obj != NULL)
    {
      //recursive_hide(obj,true);
      if (!lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN))
      {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        //lv_obj_set_local_style_prop(obj, LV_STYLE_OPA, 0, 0);   
        //lv_obj_invalidate(screenbase);      
      }
    }
  }



  obj = lv_obj_get_child(screenbase, contentindexnumber);

  if (obj != NULL)
  {
    //recursive_hide(obj,false);
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);        
    //lv_obj_remove_local_style_prop(obj, LV_STYLE_OPA, 0);    
    //lv_obj_move_foreground(obj);
    //lv_obj_invalidate(screenbase);
    //lv_refr_now(NULL);
 
  }

  return (obj);
}

lv_obj_t * GetInfoObject(void)
{
  if (screenbase == NULL) return (NULL);
  lv_obj_t * nav = lv_obj_get_child(screenbase, 0); // Get the navigation object
  lv_obj_t * obj = lv_obj_get_child(nav, 1); // Get the info label
  if (!lv_obj_check_type(obj, &lv_label_class)) return (NULL);
  return obj;    
}

lv_obj_t * GetButtonLabelObject(void)
{
  if (screenbase == NULL) return (NULL);
  lv_obj_t * nav = lv_obj_get_child(screenbase, 0);
  lv_obj_t * btn = lv_obj_get_child(nav, 0);
  if (!lv_obj_check_type(btn, &lv_button_class)) return (NULL);
  lv_obj_t * obj = lv_obj_get_child(btn, 0);  
  if (!lv_obj_check_type(obj, &lv_label_class)) return (NULL);
  return obj;    
}

lv_event_cb_t GetEvent(void)
{
  return (event);
}

lv_obj_t * GetKeyboard(void)
{
  return (keyboardbase);
}

void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    /*The original target of the event. Can be the buttons or the container*/
    lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
    /*The current target is always the container as the event is added to it*/
    lv_obj_t * cont = (lv_obj_t *)lv_event_get_current_target(e);

    lv_obj_t * parent = (lv_obj_t *)lv_obj_get_parent(ta);

    /*If container was clicked do nothing*/
    //if(ta == cont) return;

    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_active()) != LV_INDEV_TYPE_KEYPAD)
        {
            lv_align_t alignment = (lv_align_t)(uintptr_t)lv_obj_get_user_data(ta);
            lv_obj_align(kb, alignment, 0, 0);      
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(kb);
            //lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
            lv_indev_wait_release((lv_indev_t *)lv_event_get_param(e));
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(kb, LV_ALIGN_DEFAULT, 0, 0);
        lv_indev_reset(NULL, ta);
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
      lv_keyboard_set_textarea(kb, NULL);
      lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
      lv_obj_align(kb, LV_ALIGN_DEFAULT, 0, 0);
      lv_obj_clear_state(ta, LV_STATE_FOCUSED);
      lv_indev_reset(NULL, ta);
    }
}

void ObjectDeleteAndNull(lv_obj_t ** target)
{
  if (*target != NULL)
  {
    lv_lock();
    lv_obj_delete(*target);
    *target = NULL;
    lv_unlock();
  }
}

void fade_out_cb(void *obj, int32_t v) {
  if (obj) {
    lv_obj_set_style_opa(obj, v, LV_PART_MAIN);
  }
}

void fade_in_cb(void *obj, int32_t v) {
  if (obj) {
    lv_obj_set_style_opa(obj, v, LV_PART_MAIN);
  }
}

void ready_cb(lv_anim_t *anim)
{
  lv_obj_t * user = lv_anim_get_user_data(anim);  
  if (user != NULL)
  {
    lv_obj_add_flag(user, LV_OBJ_FLAG_HIDDEN);
    //lv_obj_set_style_bg_opa(user, LV_OPA_TRANSP, 0);  
  }
}

void NumpadShowLabel(lv_obj_t * label)
{
  lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)fade_in_cb);    
  lv_anim_set_completed_cb(&a, NULL);    
  lv_obj_t * user = lv_anim_get_user_data(&a);  
  if (user != NULL)
  {
    lv_obj_remove_flag(user, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t * ta = lv_obj_get_child(user, 0);
    if (ta != NULL)
    {
      lv_obj_t * ta_label = lv_textarea_get_label(ta);
      lv_label_set_text(ta_label, lv_label_get_text(label));
    }
    lv_obj_t * kb = lv_obj_get_child(user, 1);
    if (kb != NULL)
    {
      lv_obj_set_user_data(kb, label);      
    }
  }
  lv_anim_start(&a);
}

void NumpadHide()
{
  lv_obj_t * ta = NULL;
  lv_obj_t * ta_label = NULL;
  lv_obj_t * kb = NULL;
  lv_obj_t * targetlabel = NULL;

  lv_obj_t * user = lv_anim_get_user_data(&a);  
  if (user != NULL)
  {
    ta = lv_obj_get_child(user, 0);
    if (ta != NULL)
    {
      ta_label = lv_textarea_get_label(ta);
    }
    kb = lv_obj_get_child(user, 1);
    if (kb != NULL)
    {
      targetlabel = lv_obj_get_user_data(kb);
    }    
  }

  if (targetlabel != NULL)
  {
    if (ta_label != NULL)
    {
      lv_label_set_text(targetlabel, lv_label_get_text(ta_label));
    }
    if (kb != NULL)
    {
      lv_obj_set_user_data(kb, lv_obj_get_user_data(targetlabel));  
    }
  }

  lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)fade_out_cb);    
  lv_anim_set_completed_cb(&a, ready_cb);  
  lv_anim_start(&a);
}