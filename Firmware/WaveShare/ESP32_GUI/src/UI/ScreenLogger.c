#include "Screenbase.h"

#define MAX_LOG_LINES   100          /* keep only the newest N lines */

lv_obj_t * screenlogger = NULL;
static lv_obj_t * ta_logger = NULL;

lv_obj_t * mylog_create(lv_obj_t * parent)
{
    /* Main scrollable container */
    lv_obj_t * log_cont = lv_obj_create(parent);

    lv_obj_set_size(log_cont, lv_pct(100), lv_pct(100));
    lv_obj_align(log_cont, LV_ALIGN_TOP_MID, 0, 0);

    /* Make it a vertical flex column */
    lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(log_cont, 2, 0);   /* small gap between lines */
    lv_obj_set_style_pad_all(log_cont, 6, 0);   /* inner padding */

    /* Optional dark “terminal” look */
    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_color(log_cont, lv_color_hex(0x444444), 0);
    lv_obj_set_style_radius(log_cont, 4, 0);

    /* Scrollbar only when needed */
    lv_obj_set_scrollbar_mode(log_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(log_cont, LV_DIR_VER);

    return log_cont;
}

void mylog_add(const char * txt)
{
    lv_obj_t * log_cont = ta_logger;

    if (log_cont == NULL || txt == NULL) return;

    /* Create a new label for this line */
    lv_obj_t * lab = lv_label_create(log_cont);
    lv_label_set_text(lab, txt);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_WRAP);   /* wrap long lines */
    lv_obj_set_width(lab, lv_pct(100));                /* full width of container */

    /* Optional styling */
    lv_obj_set_style_text_color(lab, lv_color_hex(0x00ff00), 0);  /* green terminal text */
    // lv_obj_set_style_text_font(lab, &lv_font_montserrat_12, 0);

    /* Limit number of lines – delete oldest */
    while (lv_obj_get_child_cnt(log_cont) > MAX_LOG_LINES) {
        lv_obj_t * oldest = lv_obj_get_child(log_cont, 0);
        lv_obj_del(oldest);
    }

    /* Keep the newest line visible */
    lv_obj_scroll_to_view(lab, LV_ANIM_OFF);   /* or LV_ANIM_ON for smooth scroll */
}

void mylog_clear(void)
{
    lv_obj_t * log_cont = ta_logger;
    lv_obj_clean(log_cont);   /* deletes all children */
}

static void btn_event_cb_local(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * btn = lv_event_get_current_target_obj(e);

  if (btn != NULL)
  {
    if (code == LV_EVENT_CLICKED)
    {
      if (ta_logger != NULL)
      {
        //lv_textarea_set_text(ta_logger, "");
        mylog_clear();
     }
    }
  }
}

static void btn_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * user = lv_event_get_user_data(e);  
  /*The original target of the event. Can be the buttons or the container*/
  lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);

  if ( (btn != NULL) && (code == LV_EVENT_CLICKED) )
  {
    lv_event_cb_t event_cb = GetEvent();
    event_cb(e);
  }
}

static void Setup_Screen(lv_obj_t * cont)
{

  if (lv_obj_get_child_count(cont) == 0)
  {
    ta_logger = mylog_create(cont);
  }
}

void Setup_ScreenLogger(byte index, bool show)
{
  lv_obj_t * obj = NULL;

  if (show)
  {
    obj = GetInfoObject();
    if (obj != NULL) lv_label_set_text(obj,"Logger");

    obj = GetButtonLabelObject();
    if (obj != NULL) lv_label_set_text(obj,"Clear");
  }

  SetContentObject(screenlogger,show);
  if (screenlogger != NULL) Setup_Screen(screenlogger);

  if (show)
  {
    obj = GetButtonLabelObject();
    if (obj != NULL)
    {
      // Get the navigation button itself
      obj = lv_obj_get_parent(obj);
      // Set new color
      lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // Red border
      //lv_obj_set_style_bg_color(obj, lv_palette_darken(LV_PALETTE_RED,3), LV_PART_MAIN | LV_STATE_CHECKED);             
      // Remove standard event
      lv_event_cb_t event_cb = GetEvent();
      lv_obj_remove_event_cb(obj, event_cb);
      // Add a local event
      customevent = lv_obj_add_event_cb(obj, btn_event_cb_local, LV_EVENT_CLICKED, NULL);
    }
  }
}

void ScreenLogger_Add(const char *txt, bool newline)
{
  if (ta_logger != NULL)
  {
    mylog_add(txt);
    //lv_textarea_add_text(ta_logger, txt); 
    //if (newline) lv_textarea_add_text(ta_logger, "\n");
  }
}

int ScreenLogger_Add_Fmt(const char *format, ...)
{
  if (ta_logger == NULL)
    return -1;

  char myString[256];

  va_list args;
  va_start(args, format);

  int result = vsnprintf(myString, sizeof(myString), format, args);

  va_end(args);                    // Clean up

  if (result < 0) {
    // encoding error
    return result;
  }  

  // Optional: detect truncation
  if (result >= (int)sizeof(myString)) {
    // message was truncated
  }  

  mylog_add(myString);

  //lv_textarea_add_text(ta_logger, myString);

  return result;
}
