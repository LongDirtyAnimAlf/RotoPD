#include "Screenbase.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAX_LOG_LINES   100
#define LOG_LINE_LEN    64

lv_obj_t * screenlogger = NULL;

static lv_obj_t * log_cont = NULL;
static lv_obj_t * log_labels[MAX_LOG_LINES];

/* Fixed text storage – one buffer per line, never freed */
static char      log_text[MAX_LOG_LINES][LOG_LINE_LEN];
static uint16_t  log_count = 0;

/* ---------- Create container + fixed pool (once) ---------- */
lv_obj_t * mylog_create(lv_obj_t * parent)
{
    log_cont = lv_obj_create(parent);

    lv_obj_set_style_radius(log_cont, 0, 0);
    lv_obj_set_size(log_cont, lv_pct(100), lv_pct(100));
    lv_obj_align(log_cont, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(log_cont, 2, 0);
    lv_obj_set_style_pad_all(log_cont, 6, 0);

    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_color(log_cont, lv_color_hex(0x444444), 0);

    lv_obj_set_scrollbar_mode(log_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(log_cont, LV_DIR_VER);

    for (uint16_t i = 0; i < MAX_LOG_LINES; i++) {
        log_labels[i] = lv_label_create(log_cont);
        lv_label_set_long_mode(log_labels[i], LV_LABEL_LONG_WRAP);
        lv_obj_set_width(log_labels[i], lv_pct(100));
        lv_obj_set_style_text_color(log_labels[i], lv_color_hex(0x00ff00), 0);

        /* Point the label at its permanent buffer (empty at start) */
        log_text[i][0] = '\0';
        lv_label_set_text_static(log_labels[i], log_text[i]);

        lv_obj_add_flag(log_labels[i], LV_OBJ_FLAG_HIDDEN);
    }

    log_count = 0;
    return log_cont;
}

/* ---------- Add line – recycle object + overwrite its fixed buffer ---------- */
void mylog_add(const char * txt)
{
    if (log_cont == NULL || txt == NULL) return;

    lv_obj_t * lab;
    char *    buf;

    if (log_count < MAX_LOG_LINES) {
        lab = log_labels[log_count];
        buf = log_text[log_count];
        log_count++;
        lv_obj_clear_flag(lab, LV_OBJ_FLAG_HIDDEN);
    } else {
        /* Recycle oldest label (child 0) */
        lab = lv_obj_get_child(log_cont, 0);

        /* Find which buffer belongs to this label */
        uint16_t idx = 0;
        for (; idx < MAX_LOG_LINES; idx++) {
            if (log_labels[idx] == lab) break;
        }
        buf = log_text[idx];

        lv_obj_move_to_index(lab, -1);   /* move to end = newest */
    }

    /* Overwrite the fixed buffer – no malloc */
    strncpy(buf, txt, LOG_LINE_LEN - 1);
    buf[LOG_LINE_LEN - 1] = '\0';

    /* Tell the label the text changed (still the same pointer) */
    lv_label_set_text_static(lab, buf);

    lv_obj_scroll_to_view(lab, LV_ANIM_OFF);
}

/* ---------- Clear – only hide, never free ---------- */
void mylog_clear(void)
{
    if (log_cont == NULL) return;

    for (uint16_t i = 0; i < MAX_LOG_LINES; i++) {
        log_text[i][0] = '\0';
        lv_label_set_text_static(log_labels[i], log_text[i]);
        lv_obj_add_flag(log_labels[i], LV_OBJ_FLAG_HIDDEN);
    }
    log_count = 0;
}

/* ---------- Rest of your file stays almost the same ---------- */

static void btn_event_cb_local(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED && log_cont != NULL) {
        mylog_clear();
    }
}

static void btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_event_cb_t event_cb = GetEvent();
        if (event_cb) event_cb(e);
    }
}

static void Setup_Screen(lv_obj_t * cont)
{
    if (lv_obj_get_child_count(cont) == 0) {
        log_cont = mylog_create(cont);
    }
}

void Setup_ScreenLogger(byte index, bool show)
{
    lv_obj_t * obj = NULL;

    if (show) {
        obj = GetInfoObject();
        if (obj) lv_label_set_text(obj, "Logger");

        obj = GetButtonLabelObject();
        if (obj) lv_label_set_text(obj, "Clear");
    }

    SetContentObject(screenlogger, show);
    if (screenlogger) Setup_Screen(screenlogger);

    if (show) {
        obj = GetButtonLabelObject();
        if (obj) {
            obj = lv_obj_get_parent(obj);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_event_cb_t event_cb = GetEvent();
            lv_obj_remove_event_cb(obj, event_cb);
            customevent = lv_obj_add_event_cb(obj, btn_event_cb_local,
                                              LV_EVENT_CLICKED, NULL);
        }
    }
}

void ScreenLogger_Add(const char * txt, bool newline)
{
    (void)newline;
    if (log_cont) mylog_add(txt);
}

int ScreenLogger_Add_Fmt(const char * format, ...)
{
    if (log_cont == NULL) return -1;

    static char myString[LOG_LINE_LEN];   /* static is fine here */

    va_list args;
    va_start(args, format);
    int result = vsnprintf(myString, sizeof(myString), format, args);
    va_end(args);

    if (result < 0) return result;

    mylog_add(myString);
    return result;
}
