#include "Screenbase.h"
#include <stdio.h>

#define NUMBEROFDISPLAY 5

lv_obj_t * screen1 = NULL;

static lv_obj_t * ta1 = NULL;
static lv_obj_t * ta2 = NULL;

lv_obj_t * testdischargebutton = NULL;
lv_obj_t * startdischargebutton = NULL;
lv_obj_t * testchargebutton = NULL;
lv_obj_t * startchargebutton = NULL;
lv_obj_t * outputbutton = NULL;

lv_obj_t * zerocapacitybutton = NULL;
lv_obj_t * zeroenergybutton = NULL;
lv_obj_t * zerotimebutton = NULL;

static bool chargedisabled = true;
static bool dischargedisabled = false;

static lv_obj_t * ThresholdLed[tmLast] = {NULL};

const char * const ThresholdNames[] = {
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

/* ── Shared styles (created once) ─────────────────────────────────────────── */
static lv_style_t style_margin;
static lv_style_t style_section_border;
static lv_style_t style_grid_cell;
static lv_style_t style_unit_label;
static lv_style_t style_name_label;
static lv_style_t style_btn_checked;
static lv_style_t style_zero_btn;
static bool styles_inited = false;

static void init_styles_once(void)
{
    if (styles_inited) return;
    styles_inited = true;

    lv_style_init(&style_margin);
    lv_style_set_margin_left(&style_margin, 2);
    lv_style_set_margin_right(&style_margin, 2);
    lv_style_set_margin_top(&style_margin, 2);
    lv_style_set_margin_bottom(&style_margin, 2);

    lv_style_init(&style_section_border);
    lv_style_set_border_width(&style_section_border, 2);
    lv_style_set_border_color(&style_section_border, lv_color_hex(0xFF0000));
    lv_style_set_border_opa(&style_section_border, LV_OPA_COVER);

    lv_style_init(&style_grid_cell);
    lv_style_set_bg_color(&style_grid_cell, lv_palette_darken(LV_PALETTE_BLUE_GREY, 4));
    lv_style_set_pad_all(&style_grid_cell, 0);

    lv_style_init(&style_unit_label);
    lv_style_set_text_opa(&style_unit_label, 255);
    lv_style_set_text_font(&style_unit_label, &lv_font_montserrat_12);
    lv_style_set_text_color(&style_unit_label, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_translate_x(&style_unit_label, -8);
    lv_style_set_translate_y(&style_unit_label, -6);

    lv_style_init(&style_name_label);
    lv_style_set_text_opa(&style_name_label, 255);
    lv_style_set_text_font(&style_name_label, &lv_font_montserrat_18);
    lv_style_set_text_color(&style_name_label, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_pad_all(&style_name_label, 0);

    lv_style_init(&style_btn_checked);
    lv_style_set_bg_color(&style_btn_checked, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_init(&style_zero_btn);
    lv_style_set_bg_color(&style_zero_btn, lv_palette_darken(LV_PALETTE_RED, 4));
}

/* ── Button enable / disable helpers ──────────────────────────────────────── */

static void ChargeStatus(bool Status)
{
    if (Status) {
        lv_obj_add_state(testchargebutton, LV_STATE_DISABLED);
        lv_obj_add_state(startchargebutton, LV_STATE_DISABLED);
        lv_obj_add_state(ta2, LV_STATE_DISABLED);
        lv_obj_clear_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    } else {
        lv_obj_remove_state(testchargebutton, LV_STATE_DISABLED);
        lv_obj_remove_state(startchargebutton, LV_STATE_DISABLED);
        lv_obj_remove_state(ta2, LV_STATE_DISABLED);
        lv_obj_add_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    }
    lv_obj_remove_state(testchargebutton, LV_STATE_CHECKED);
    lv_obj_remove_state(startchargebutton, LV_STATE_CHECKED);
}

static void DischargeStatus(bool Status)
{
    if (Status) {
        lv_obj_add_state(testdischargebutton, LV_STATE_DISABLED);
        lv_obj_add_state(startdischargebutton, LV_STATE_DISABLED);
        lv_obj_add_state(ta1, LV_STATE_DISABLED);
        lv_obj_clear_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    } else {
        lv_obj_remove_state(testdischargebutton, LV_STATE_DISABLED);
        lv_obj_remove_state(startdischargebutton, LV_STATE_DISABLED);
        lv_obj_remove_state(ta1, LV_STATE_DISABLED);
        lv_obj_add_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    }
    lv_obj_remove_state(testdischargebutton, LV_STATE_CHECKED);
    lv_obj_remove_state(startdischargebutton, LV_STATE_CHECKED);
}

static void setbuttons(lv_obj_t * btn, bool checked)
{
    static lv_obj_t ** btns[5] = {
        &testdischargebutton, &startdischargebutton,
        &testchargebutton, &startchargebutton, &outputbutton
    };

    if (checked) {
        lv_obj_add_state(btn, LV_STATE_CHECKED);

        if (btn != outputbutton) {
            lv_obj_add_state(outputbutton, LV_STATE_CHECKED);

            for (byte i = 0; i < 5; i++) {
                lv_obj_t * obj = *btns[i];
                if (obj != NULL && obj != btn) {
                    lv_obj_add_state(obj, LV_STATE_DISABLED);
                    lv_obj_add_state(zerocapacitybutton, LV_STATE_DISABLED);
                    lv_obj_add_state(zeroenergybutton, LV_STATE_DISABLED);
                    if (zerotimebutton != NULL)
                        lv_obj_add_state(zerotimebutton, LV_STATE_DISABLED);
                    lv_obj_add_state(ta1, LV_STATE_DISABLED);
                    lv_obj_add_state(ta2, LV_STATE_DISABLED);
                    lv_obj_clear_flag(ta1, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                    lv_obj_clear_flag(ta2, LV_OBJ_FLAG_CLICK_FOCUSABLE);
                }
            }
        }
    } else {
        lv_obj_remove_state(outputbutton, LV_STATE_CHECKED);
        lv_obj_remove_state(outputbutton, LV_STATE_DISABLED);

        ChargeStatus(chargedisabled);
        DischargeStatus(dischargedisabled);
        lv_obj_remove_state(zerocapacitybutton, LV_STATE_DISABLED);
        lv_obj_remove_state(zeroenergybutton, LV_STATE_DISABLED);
        if (zerotimebutton != NULL)
            lv_obj_remove_state(zerotimebutton, LV_STATE_DISABLED);
    }
}

/* ── Event callbacks ──────────────────────────────────────────────────────── */

static void btn_event_cb(lv_event_t * e)
{
    static bool longpress = false;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_current_target_obj(e);
    lv_obj_t * user = lv_event_get_user_data(e);

    if (btn == NULL)
        return;

    if (code == LV_EVENT_CLICKED) {
        lv_event_cb_t event_cb = GetEvent();
        if (event_cb) event_cb(e);
    }

    if (!longpress && code == LV_EVENT_LONG_PRESSED) {
        longpress = true;
        bool checked = (lv_obj_get_state(btn) & LV_STATE_CHECKED);
        setbuttons(btn, !checked);
        lv_obj_send_event(btn, LV_EVENT_VALUE_CHANGED, user);
    }

    if (longpress && code == LV_EVENT_VALUE_CHANGED) {
        lv_event_cb_t event_cb = GetEvent();
        if (event_cb) event_cb(e);
    }

    if (code == LV_EVENT_RELEASED)
        longpress = false;
}

void keyboard_event_cb(lv_event_t * e)
{
    lv_event_cb_t event_cb = GetEvent();
    if (event_cb) event_cb(e);
}

static void ta_event_cb_local(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t * label = lv_event_get_target(e);
        NumpadShowLabel(label);
    }
}

/* ── Screen construction ──────────────────────────────────────────────────── */

void Setup_Screen1(byte index)
{
    lv_obj_t * obj;
    lv_obj_t * label;
    lv_obj_t * cell;
    lv_obj_t * grid;
    lv_obj_t * cont;
    lv_obj_t * ta;
    lv_obj_t * btn;

    obj = GetInfoObject();
    if (obj != NULL)
        lv_label_set_text(obj, "DATA");

    obj = GetButtonLabelObject();
    if (obj != NULL)
        lv_label_set_text(obj, "Chart");

    SetContentObject(screen1, true);
    cont = screen1;

    if (cont == NULL || lv_obj_get_child_count(cont) > 0)
        return;

    init_styles_once();

    /* ── Top section ──────────────────────────────────────────────────── */
    lv_obj_t * top_cont = lv_obj_create(cont);
    lv_obj_remove_style_all(top_cont);
    lv_obj_set_size(top_cont, lv_pct(100), lv_pct(18));
    lv_obj_align(top_cont, LV_ALIGN_TOP_MID, 0, 0);

    /* Discharge section */
    obj = lv_obj_create(top_cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, lv_pct(40), lv_pct(100));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_add_style(obj, &style_margin, 0);
    lv_obj_add_style(obj, &style_section_border, 0);

    label = lv_label_create(obj);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "Discharge set [mA]");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t * disc_row = lv_obj_create(obj);
    lv_obj_remove_style_all(disc_row);
    lv_obj_set_size(disc_row, lv_pct(100), lv_pct(70));
    lv_obj_align(disc_row, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(disc_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(disc_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn = lv_button_create(disc_row);
    lv_obj_set_size(btn, lv_pct(35), LV_SIZE_CONTENT);
    lv_obj_add_style(btn, &style_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    testdischargebutton = btn;

    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_center(label);
    lv_label_set_text(label, "TEST");

    ta = lv_label_create(disc_row);
    lv_obj_add_style(ta, &input_label_style, 0);
    lv_label_set_text(ta, "0");
    lv_obj_set_size(ta, lv_pct(60), LV_SIZE_CONTENT);
    lv_obj_add_flag(ta, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_label_set_long_mode(ta, LV_LABEL_LONG_MODE_CLIP);    
    lv_obj_add_event_cb(ta, ta_event_cb_local, LV_EVENT_CLICKED, NULL);
    ta1 = ta;
    lv_obj_set_user_data(ta1, testdischargebutton);

    /* Charge section */
    obj = lv_obj_create(top_cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, lv_pct(40), lv_pct(100));
    lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_add_style(obj, &style_margin, 0);
    lv_obj_add_style(obj, &style_section_border, 0);

    label = lv_label_create(obj);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_text(label, "Charge set [mA]");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t * chg_row = lv_obj_create(obj);
    lv_obj_remove_style_all(chg_row);
    lv_obj_set_size(chg_row, lv_pct(100), lv_pct(70));
    lv_obj_align(chg_row, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(chg_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chg_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ta = lv_label_create(chg_row);
    lv_obj_add_style(ta, &input_label_style, 0);
    lv_label_set_text(ta, "0");
    lv_obj_set_size(ta, lv_pct(60), LV_SIZE_CONTENT);
    lv_obj_add_flag(ta, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
    lv_label_set_long_mode(ta, LV_LABEL_LONG_MODE_CLIP);    
    lv_obj_add_event_cb(ta, ta_event_cb_local, LV_EVENT_CLICKED, NULL);
    ta2 = ta;

    btn = lv_button_create(chg_row);
    lv_obj_set_size(btn, lv_pct(35), LV_SIZE_CONTENT);
    lv_obj_add_style(btn, &style_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    testchargebutton = btn;

    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_center(label);
    lv_label_set_text(label, "TEST");

    lv_obj_set_user_data(ta2, testchargebutton);

    /* Output switch */
    obj = lv_obj_create(top_cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, lv_pct(20), lv_pct(100));
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_add_style(obj, &style_margin, 0);

    btn = lv_button_create(obj);
    lv_obj_set_size(btn, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(btn, &style_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    outputbutton = btn;

    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_center(label);
    lv_label_set_text(label, "SWITCH\nOUTPUT");

    /* ── Grid ─────────────────────────────────────────────────────────── */
    static lv_coord_t col_dsc[] = {
        LV_GRID_FR(7), LV_GRID_FR(14), LV_GRID_FR(4), LV_GRID_FR(6),
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t row_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST
    };

    grid = lv_obj_create(cont);
    lv_obj_set_size(grid, lv_pct(95), 54 * NUMBEROFDISPLAY);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(grid, 4, 0);
    lv_obj_set_style_pad_row(grid, 4, 0);
    lv_obj_set_style_pad_column(grid, 4, 0);
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    static const char * unit_names[] = { "Volt", "Amps", "W", "Wh", "s" };
    static const char * row_names[]  = { "Voltage", "Current", "Power", "Energy", "Time" };

    for (int i = 0; i < NUMBEROFDISPLAY; i++) {

        /* Value display cell (col 1) */
        cell = lv_obj_create(grid);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(cell, &style_grid_cell, 0);
        lv_obj_set_style_pad_left(cell, 10, 0);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 1, 1,
                                   LV_GRID_ALIGN_STRETCH, i, 1);
        lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(cell, 10, 0);

        byte count = 5;
        if (i == 2 || i == 3) count = 6;
        if (i == 4) count = 7;
        Display[i] = create_display(cell, lv_palette_main(LV_PALETTE_RED), false, count);

        obj = lv_label_create(cell);
        lv_obj_add_style(obj, &style_unit_label, 0);
        lv_label_set_text(obj, unit_names[i]);

        /* Name cell (col 0) */
        cell = lv_obj_create(grid);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(cell, &style_grid_cell, 0);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 0, 1,
                                   LV_GRID_ALIGN_STRETCH, i, 1);

        obj = lv_label_create(cell);
        lv_obj_add_style(obj, &style_name_label, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, row_names[i]);

        /* Zero / Auto button cell (col 2) */
        cell = lv_obj_create(grid);
        lv_obj_remove_style_all(cell);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 2, 1,
                                   LV_GRID_ALIGN_STRETCH, i, 1);
        lv_obj_set_style_pad_all(cell, 0, 0);

        obj = lv_button_create(cell);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(80));
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

        if (i == 2 || i == 3 || i == 4) {
            lv_obj_add_style(obj, &style_zero_btn, 0);

            label = lv_label_create(obj);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
            lv_obj_center(label);

            if (i == 2) {
                lv_label_set_text(label, "Zero");
                lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);
                zerocapacitybutton = obj;
            } else if (i == 3) {
                lv_label_set_text(label, "Zero");
                lv_obj_add_event_cb(obj, btn_event_cb, LV_EVENT_CLICKED, NULL);
                zeroenergybutton = obj;
            } else {
                lv_label_set_text(label, "Auto");
                zerotimebutton = obj;
                lv_obj_add_state(obj, LV_STATE_DISABLED);
            }
        } else {
            lv_obj_add_state(obj, LV_STATE_DISABLED);
        }
    }

    /* Threshold LEDs (col 3, spans all rows) */
    cell = lv_obj_create(grid);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, 3, 1,
                               LV_GRID_ALIGN_STRETCH, 0, NUMBEROFDISPLAY);
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cell, 5, 0);
    lv_obj_set_style_pad_top(cell, 10, 0);
    lv_obj_add_style(cell, &style_grid_cell, 0);
    lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cell, 14, 0);

    for (int i = (tmNONE + 1); i < tmLast; i++) {
        if (i == tmDVT || i == tmDELTAT)
            continue;

        obj = lv_led_create(cell);
        ThresholdLed[i] = obj;
        lv_led_set_brightness(obj, 255);
        lv_led_set_color(obj, lv_palette_main(LV_PALETTE_ORANGE));
        lv_led_off(obj);
        lv_obj_set_size(obj, lv_pct(100), LV_SIZE_CONTENT);

        label = lv_label_create(obj);
        lv_obj_center(label);
        lv_obj_set_style_text_opa(label, 255, 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_label_set_text(label, ThresholdNames[i]);
    }

    /* ── Bottom buttons ───────────────────────────────────────────────── */
    lv_obj_t * bottom_cont = lv_obj_create(cont);
    lv_obj_remove_style_all(bottom_cont);
    lv_obj_clear_flag(bottom_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(bottom_cont, lv_pct(100), lv_pct(15));
    lv_obj_align(bottom_cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    btn = lv_button_create(bottom_cont);
    lv_obj_set_size(btn, lv_pct(48), LV_SIZE_CONTENT);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(btn, &style_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
    startdischargebutton = btn;

    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);
    lv_label_set_text(label, "GO DISCHARGE");

    btn = lv_button_create(bottom_cont);
    lv_obj_set_size(btn, lv_pct(48), LV_SIZE_CONTENT);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(btn, &style_btn_checked, LV_PART_MAIN | LV_STATE_CHECKED);
    startchargebutton = btn;

    label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);
    lv_label_set_text(label, "GO CHARGE");
}

/* ── Public update API (unchanged behaviour) ──────────────────────────────── */

void Screen1SetThresholdLedEnabled(TThresholdModes Mode, bool On)
{
    lv_obj_t * obj = ThresholdLed[Mode];
    if (obj == NULL) return;

    obj = lv_obj_get_child(obj, 0);
    if (On)
        lv_obj_set_style_text_color(obj, lv_palette_darken(LV_PALETTE_YELLOW, 1), 0);
    else
        lv_obj_set_style_text_color(obj, lv_color_black(), 0);
}

void Screen1SetThresholdLed(TThresholdModes Mode, bool On)
{
    lv_obj_t * obj = ThresholdLed[Mode];
    if (obj == NULL) return;
    if (On) lv_led_on(obj);
    else    lv_led_off(obj);
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

void Screen1SetOutput(bool On)
{
    if (On) lv_obj_add_state(outputbutton, LV_STATE_CHECKED);
    else    lv_obj_remove_state(outputbutton, LV_STATE_CHECKED);
}

void Screen1SetData(PBatterySetting SET)
{
    Settings = SET;

#ifndef STANDALONE
    lv_obj_add_state(ta1, LV_STATE_DISABLED);
    lv_obj_add_state(ta2, LV_STATE_DISABLED);
    lv_obj_add_state(testdischargebutton, LV_STATE_DISABLED);
    lv_obj_add_state(startdischargebutton, LV_STATE_DISABLED);
    lv_obj_add_state(testchargebutton, LV_STATE_DISABLED);
    lv_obj_add_state(startchargebutton, LV_STATE_DISABLED);
    lv_obj_add_state(outputbutton, LV_STATE_DISABLED);
    lv_obj_add_state(zerocapacitybutton, LV_STATE_DISABLED);
    lv_obj_add_state(zeroenergybutton, LV_STATE_DISABLED);
    if (zerotimebutton != NULL)
        lv_obj_add_state(zerotimebutton, LV_STATE_DISABLED);
#else
    chargedisabled   = (SET->Stages[FIXEDCHARGESTAGENUMBER].Status == smDisabled);
    dischargedisabled = (SET->Stages[FIXEDDISCHARGESTAGENUMBER].Status == smDisabled);

    ChargeStatus(chargedisabled);
    DischargeStatus(dischargedisabled);

    PRunDatas RDS = &SET->TestData.RunDatas;
    Screen1AddVIData(RDS->LastBatteryData.V, RDS->LastBatteryData.I);
    Screen1AddEPData(RDS->Energy / 1000, RDS->LastBatteryData.P);
    Screen1AddTData(RDS->Time);

    if (SET->TestData.Active == bmActive) {
        if (SET->TestData.SetStageMode == smCurrent) setbuttons(startdischargebutton, true);
        if (SET->TestData.SetStageMode == smCharge)  setbuttons(startchargebutton, true);
    } else {
        if (SET->TestData.SetStageMode == smCurrent) setbuttons(testdischargebutton, true);
        if (SET->TestData.SetStageMode == smCharge)  setbuttons(testchargebutton, true);
    }

    Screen1SetOutput(SET->TestData.SetStageMode != smOff &&
                     SET->TestData.SetStageMode != smZero &&
                     SET->TestData.SetStageMode != smDisabled);
#endif

    char str[30];
    dword Value;

    Value = 0;
#ifdef STANDALONE
    Value = SET->Stages[FIXEDDISCHARGESTAGENUMBER].SetValue;
#else
    if (SET->TestData.SetStageMode == smCurrent)
        Value = SET->TestData.SetStageValue;
#endif
    sprintf(str, "%lu", (unsigned long)Value);
    lv_label_set_text(ta1, str);

    Value = 0;
#ifdef STANDALONE
    Value = SET->Stages[FIXEDCHARGESTAGENUMBER].SetValue;
#else
    if (SET->TestData.SetStageMode == smCharge)
        Value = SET->TestData.SetStageValue;
#endif
    sprintf(str, "%lu", (unsigned long)Value);
    lv_label_set_text(ta2, str);

    for (byte i = (tmNONE + 1); i < tmLast; i++) {
        Screen1SetThresholdLed((TThresholdModes)i,
                               SET->TestData.RunDatas.ThresholdResult[i].Triggered);
#ifdef STANDALONE
        bool Enabled = false;
        Enabled |= (SET->TestData.SetStageMode == smCurrent &&
                    SET->Stages[FIXEDDISCHARGESTAGENUMBER].ThresholdSettings[i].Enabled);
        Enabled |= (SET->TestData.SetStageMode == smCharge &&
                    SET->Stages[FIXEDCHARGESTAGENUMBER].ThresholdSettings[i].Enabled);
        Screen1SetThresholdLedEnabled((TThresholdModes)i, Enabled);
#endif
    }
}
