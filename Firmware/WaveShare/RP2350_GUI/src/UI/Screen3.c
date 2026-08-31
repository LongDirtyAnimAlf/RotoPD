#include "Screenbase.h"

// ── PDO type constants ────────────────────────────────────────────────────────
#define PDO_TYPE_FIXED       (0)
#define PDO_TYPE_PPS         (1)   // type=1 in SPR slot (index 1–7)
#define PDO_TYPE_AVS         (2)   // type=1 in EPR slot (index 8–13)

#define CHARGECOLOR          LV_PALETTE_GREEN
#define DISCHARGECOLOR       LV_PALETTE_RED
#define INITCOLOR            LV_PALETTE_GREY

#define MAX_PDO_ENTRIES      (13)

lv_obj_t * screen3 = NULL;
lv_obj_t * getpdolistbutton = NULL;

static lv_obj_t * pdo_list_cont = NULL;
static lv_obj_t * PDOCells[MAX_PDO_ENTRIES] = {NULL};

/* Shared styles – initialised only once */
static lv_style_t style_cell;
static lv_style_t style_label;
static lv_style_t style_value;
static lv_style_t style_btn;
static lv_style_t style_btn_label;
static bool styles_inited = false;

static void init_styles_once(void)
{
    if (styles_inited) return;
    styles_inited = true;

    /* Cell container */
    lv_style_init(&style_cell);
    lv_style_set_bg_color(&style_cell, lv_palette_darken(LV_PALETTE_INDIGO, 4));
    lv_style_set_border_color(&style_cell, lv_palette_darken(LV_PALETTE_BLUE_GREY, 4));
    lv_style_set_border_width(&style_cell, 2);
    lv_style_set_pad_top(&style_cell, 2);
    lv_style_set_pad_bottom(&style_cell, 2);
    lv_style_set_pad_column(&style_cell, 10);
    lv_style_set_radius(&style_cell, 0);
    /* No shadow – expensive */

    /* Small text labels (SPR/EPR, type) */
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_14);
    lv_style_set_pad_all(&style_label, 4);
    lv_style_set_radius(&style_label, 0);

    /* Value labels (voltage / current) */
    lv_style_init(&style_value);
    lv_style_set_text_font(&style_value, &lv_font_montserrat_14);
    lv_style_set_border_width(&style_value, 2);
    lv_style_set_radius(&style_value, 0);
    lv_style_set_pad_top(&style_value, 4);
    lv_style_set_pad_bottom(&style_value, 4);
    lv_style_set_pad_left(&style_value, 4);
    lv_style_set_pad_right(&style_value, 4);

    /* Select button */
    lv_style_init(&style_btn);
    lv_style_set_border_width(&style_btn, 2);
    lv_style_set_pad_top(&style_btn, 8);
    lv_style_set_pad_bottom(&style_btn, 8);
    lv_style_set_pad_left(&style_btn, 6);
    lv_style_set_pad_right(&style_btn, 6);
    lv_style_set_radius(&style_btn, 0);

    /* Label inside the Select button */
    lv_style_init(&style_btn_label);
    lv_style_set_text_font(&style_btn_label, &lv_font_montserrat_14);
}

static lv_obj_t * CreatePDOCell(lv_obj_t * parent);
void Screen3SetPDO(
    uint8_t  index,
    bool     valid,
    bool     isEPR,
    uint8_t  type,
    uint16_t minVoltage_mV,
    uint16_t maxVoltage_mV,
    uint16_t maxCurrent_mA);

void Screen3ClearPDOList(void)
{
    for (uint8_t i = 0; i < MAX_PDO_ENTRIES; i++) {
        lv_obj_t * cell = PDOCells[i];
        if (cell != NULL) {
            if (!lv_obj_has_flag(cell, LV_OBJ_FLAG_HIDDEN))
                lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_user_data(cell, NULL);
        }
    }
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);

    if (btn == NULL || code != LV_EVENT_CLICKED)
        return;

    if (btn == getpdolistbutton) {
        Screen3ClearPDOList();
    } else {
        lv_obj_t * cont = lv_event_get_current_target(e);
        if (btn == cont) return;   /* clicked on container itself */
    }

    lv_event_cb_t event_cb = GetEvent();
    if (event_cb)
        event_cb(e);
}

static lv_obj_t * CreatePDOCell(lv_obj_t * parent)
{
    init_styles_once();

    lv_obj_t * cell = lv_obj_create(parent);
    lv_obj_remove_style_all(cell);
    lv_obj_add_style(cell, &style_cell, 0);
    lv_obj_add_flag(cell, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cell, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Child 0 – SPR / EPR */
    lv_obj_t * label = lv_label_create(cell);
    lv_obj_set_width(label, 40);
    lv_label_set_text(label, "-");
    lv_obj_add_style(label, &style_label, 0);

    /* Child 1 – type (Fixed / PPS / AVS) */
    label = lv_label_create(cell);
    lv_obj_set_width(label, 60);
    lv_label_set_text(label, "-");
    lv_obj_add_style(label, &style_label, 0);

    /* Child 2 – voltage */
    lv_obj_t * ta = lv_label_create(cell);
    lv_obj_set_width(ta, 150);
    lv_label_set_text(ta, "--- Volts");
    lv_obj_add_style(ta, &style_value, 0);
    lv_obj_set_style_border_color(ta, lv_palette_main(INITCOLOR), 0);
    lv_obj_set_style_bg_color(ta, lv_palette_lighten(INITCOLOR, 4), 0);
    lv_obj_set_style_text_color(ta, lv_palette_darken(INITCOLOR, 4), LV_PART_SELECTED);

    /* Child 3 – current */
    ta = lv_label_create(cell);
    lv_obj_set_width(ta, 90);
    lv_label_set_text(ta, "--- Amps");
    lv_obj_add_style(ta, &style_value, 0);
    lv_obj_set_style_border_color(ta, lv_palette_main(INITCOLOR), 0);
    lv_obj_set_style_bg_color(ta, lv_palette_lighten(INITCOLOR, 4), 0);
    lv_obj_set_style_text_color(ta, lv_palette_darken(INITCOLOR, 4), LV_PART_SELECTED);

    /* Child 4 – Select button */
    lv_obj_t * cell_button = lv_button_create(cell);
    lv_obj_add_flag(cell_button, LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_FLOATING);
    lv_obj_set_size(cell_button, LV_SIZE_CONTENT, lv_pct(90));
    lv_obj_align(cell_button, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_add_style(cell_button, &style_btn, 0);
    lv_obj_set_style_border_color(cell_button, lv_palette_main(INITCOLOR), 0);
    lv_obj_set_style_bg_color(cell_button, lv_palette_darken(INITCOLOR, 4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cell_button, lv_palette_darken(INITCOLOR, 100), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_t * btn_label = lv_label_create(cell_button);
    lv_label_set_text(btn_label, "Select");
    lv_obj_add_style(btn_label, &style_btn_label, 0);
    lv_obj_center(btn_label);

    /* Start hidden – shown only when a valid PDO is assigned */
    lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN);

    return cell;
}

static void Setup_Screen(lv_obj_t * cont)
{
    if (lv_obj_get_child_count(cont) > 0)
        return;   /* already built */

    init_styles_once();

    /* ── Refresh button ─────────────────────────────────────────────── */
    lv_obj_t * btn = lv_button_create(cont);
    getpdolistbutton = btn;
    lv_obj_set_size(btn, LV_SIZE_CONTENT, lv_pct(15));
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn, lv_palette_darken(LV_PALETTE_RED, 3), 0);
    lv_obj_set_style_pad_top(btn, 14, 0);
    lv_obj_set_style_pad_bottom(btn, 14, 0);
    lv_obj_set_style_margin_top(btn, 14, 0);
    lv_obj_set_style_margin_bottom(btn, 14, 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_label_set_text(label, "Click to refresh PDOs");
    lv_obj_center(label);

    /* ── PDO list container ─────────────────────────────────────────── */
    pdo_list_cont = lv_obj_create(cont);
    lv_obj_set_size(pdo_list_cont, lv_pct(100), lv_pct(80));
    lv_obj_align_to(pdo_list_cont, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_scrollbar_mode(pdo_list_cont, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_scroll_dir(pdo_list_cont, LV_DIR_VER);
    lv_obj_set_flex_flow(pdo_list_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(pdo_list_cont, 10, 0);
    lv_obj_add_event_cb(pdo_list_cont, btn_event_cb, LV_EVENT_CLICKED, screen3);

    /* ── Create all cells once (hidden) ─────────────────────────────── */
    for (int i = 0; i < MAX_PDO_ENTRIES; i++) {
        PDOCells[i] = CreatePDOCell(pdo_list_cont);
    }
}

void Setup_Screen3(byte index, bool show)
{
    if (show) {
        lv_obj_t * obj = GetInfoObject();
        if (obj != NULL)
            lv_label_set_text(obj, "PDO-list");

        obj = GetButtonLabelObject();
        if (obj != NULL)
            lv_label_set_text(obj, "Logger");
    }

    SetContentObject(screen3, show);
    if (screen3 != NULL)
        Setup_Screen(screen3);
}

void Screen3SetPDO(
    uint8_t  index,          /* 1-based (1–13) */
    bool     valid,          /* detect bit = 1 */
    bool     isEPR,          /* true for index 8–13 */
    uint8_t  type,           /* PDO_TYPE_FIXED / _PPS / _AVS */
    uint16_t minVoltage_mV,
    uint16_t maxVoltage_mV,
    uint16_t maxCurrent_mA)
{
    if (index == 0)
        return;

    lv_obj_t * cell = NULL;

    if (valid) {
        /* Find first free cell */
        for (uint8_t i = 0; i < MAX_PDO_ENTRIES; i++) {
            cell = PDOCells[i];
            uint8_t pdo_index = (uint8_t)(uintptr_t)lv_obj_get_user_data(cell);
            if (pdo_index == 0)
                break;
            cell = NULL;
        }
    }

    if (cell == NULL)
        return;

    lv_palette_t p = isEPR ? DISCHARGECOLOR : CHARGECOLOR;

    if (!valid) {
        if (!lv_obj_has_flag(cell, LV_OBJ_FLAG_HIDDEN))
            lv_obj_add_flag(cell, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_user_data(cell, NULL);
        return;
    }

    /* Show cell and store index */
    if (lv_obj_has_flag(cell, LV_OBJ_FLAG_HIDDEN))
        lv_obj_remove_flag(cell, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_user_data(cell, (void *)(uintptr_t)index);

    /* Child 0 – SPR / EPR */
    lv_obj_t * obj = lv_obj_get_child(cell, 0);
    if (obj && lv_obj_check_type(obj, &lv_label_class))
        lv_label_set_text(obj, isEPR ? "EPR" : "SPR");

    /* Child 1 – type */
    obj = lv_obj_get_child(cell, 1);
    if (obj && lv_obj_check_type(obj, &lv_label_class)) {
        switch (type) {
            case PDO_TYPE_FIXED: lv_label_set_text(obj, "Fixed"); break;
            case PDO_TYPE_PPS:   lv_label_set_text(obj, "PPS");   break;
            case PDO_TYPE_AVS:   lv_label_set_text(obj, "AVS");   break;
            default:             lv_label_set_text(obj, "-");     break;
        }
    }

    /* Child 2 – voltage */
    obj = lv_obj_get_child(cell, 2);
    if (obj && lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), 0);
        lv_obj_set_style_bg_color(obj, lv_palette_lighten(p, 4), 0);
        lv_obj_set_style_text_color(obj, lv_palette_darken(p, 4), LV_PART_SELECTED);

        int cv = (maxVoltage_mV + 5) / 10;
        int whole = cv / 100;
        int hundredths = cv % 100;

        if (type == PDO_TYPE_FIXED) {
            lv_label_set_text_fmt(obj, "%d.%02d Volts", whole, hundredths);
        } else {
            int cv_min = (minVoltage_mV + 5) / 10;
            int min_whole = cv_min / 100;
            int min_hund  = cv_min % 100;
            lv_label_set_text_fmt(obj, "%d.%02d - %d.%02d Volts",
                                  min_whole, min_hund, whole, hundredths);
        }
    }

    /* Child 3 – current */
    obj = lv_obj_get_child(cell, 3);
    if (obj && lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), 0);
        lv_obj_set_style_bg_color(obj, lv_palette_lighten(p, 4), 0);
        lv_obj_set_style_text_color(obj, lv_palette_darken(p, 4), LV_PART_SELECTED);

        int cv = (maxCurrent_mA + 5) / 10;
        int whole = cv / 100;
        int hundredths = cv % 100;
        lv_label_set_text_fmt(obj, "%d.%02d Amps", whole, hundredths);
    }

    /* Child 4 – Select button */
    obj = lv_obj_get_child(cell, 4);
    if (obj && lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_set_style_border_color(obj, lv_palette_main(p), 0);
        lv_obj_set_style_bg_color(obj, lv_palette_darken(p, 4), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(obj, lv_palette_darken(p, 100), LV_PART_MAIN | LV_STATE_DISABLED);
        lv_obj_set_user_data(obj, (void *)(uintptr_t)index);
    }
}
