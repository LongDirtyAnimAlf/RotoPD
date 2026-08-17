#include "Screenbase.h"

lv_obj_t * screenlogger = NULL;
static lv_obj_t * ta_logger = NULL;

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
    ta_logger = lv_textarea_create(cont);
    lv_obj_set_size(ta_logger, lv_pct(100), lv_pct(100));
    lv_obj_align(ta_logger, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(ta_logger, 0, LV_PART_MAIN);
    //lv_obj_add_state(ta_logger, LV_STATE_DISABLED);
    lv_obj_remove_style(ta_logger, NULL, LV_PART_CURSOR);
    lv_textarea_set_cursor_pos(ta_logger, LV_TEXTAREA_CURSOR_LAST);
    lv_textarea_add_text(ta_logger, "USB logger\n");
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
    if (obj != NULL)
    {
      lv_label_set_text(obj,"---");
      // Get the navigation button itself
      //btn_next_state = (uint16_t)((uint8_t)SCREENINDEX << 8 | (uint8_t)btn_next);    
    }
  }
  SetContentObject(screenlogger,show);
  if (screenlogger != NULL) Setup_Screen(screenlogger);
}

void ScreenLogger_Add(const char *txt, bool newline)
{
 if (ta_logger != NULL)
 {
  lv_textarea_add_text(ta_logger, txt); 
  if (newline) lv_textarea_add_text(ta_logger, "\n");
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

  lv_textarea_add_text(ta_logger, myString);

  return result;
}
