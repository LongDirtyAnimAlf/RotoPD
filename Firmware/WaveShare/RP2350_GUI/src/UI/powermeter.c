#include "lvgl.h"
#include "Screenbase.h"

#include "./../../shared.h"

extern const lv_font_t lv_font_dseg7_32;

// ============== Configuration ==============
#define GAUGE_MAX           240
#define GAUGE_REDZONE       140
#define ANGLE_RANGE         270
#define GAUGE_ROTATION      135
#define GAUGE_SIZE          300
#define ARC_WIDTH           14
#define NEEDLE_LENGTH       120
#define TIP_RADIUS          4          // size of the red tip

static lv_obj_t * scale;
static lv_obj_t * arc_bg;
static lv_obj_t * arc_glow_outer;
static lv_obj_t * arc_glow_inner;
static lv_obj_t * arc_indicator;
static lv_obj_t * needle_glow;      // thicker soft glow
static lv_obj_t * needle_line;      // sharp main needle
static lv_obj_t * label_value;
static lv_obj_t * digital_display;
static lv_obj_t * voltage_display;
static lv_obj_t * current_display;

void gauge_set_voltage(int16_t V)
{
    if (voltage_display != NULL) SetDisplaymV(voltage_display, (V));
}

void gauge_set_current(int16_t I)
{
    if (current_display != NULL) SetDisplaymV(current_display, (I));
}

void gauge_set_energy(int32_t E)
{
    if (digital_display != NULL) SetDisplaymV(digital_display, (E));    
}

// ============== Update ==============
void gauge_set_power(int32_t P)
{
    if(P < 0)         P = 0;
    if(P > GAUGE_MAX) P = GAUGE_MAX;

    lv_arc_set_value(arc_indicator, P);
    lv_arc_set_value(arc_glow_outer, P);
    lv_arc_set_value(arc_glow_inner, P);

    // Both needle layers must be updated
    lv_scale_set_line_needle_value(scale, needle_glow, NEEDLE_LENGTH + 4, P);
    lv_scale_set_line_needle_value(scale, needle_line, NEEDLE_LENGTH, P);

    //lv_label_set_text_fmt(label_value, "%d", (int)P);
    lv_label_set_text_fmt(label_value, "%03d", P);    
}

// ============== Create ==============
void create_3d_gauge(lv_obj_t * parent)
{
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_scrollable(parent, false);

    /* Outer glowing container */
    static lv_style_t style_glow;
    lv_style_init(&style_glow);
    lv_style_set_bg_opa(&style_glow, LV_OPA_COVER);
    lv_style_set_shadow_width(&style_glow, 40);
    lv_style_set_shadow_spread(&style_glow, 5);
    lv_style_set_shadow_color(&style_glow, lv_color_hex(0xFF0000));
    lv_style_set_shadow_opa(&style_glow, LV_OPA_80);
    lv_style_set_shadow_offset_x(&style_glow, 0);
    lv_style_set_shadow_offset_y(&style_glow, 0);

    lv_obj_t * cont2 = lv_obj_create(parent);
    lv_obj_set_size(cont2, GAUGE_SIZE + 80, GAUGE_SIZE + 80);
    lv_obj_set_scrollable(cont2, false);
    //lv_obj_center(cont2);
    lv_obj_align(cont2, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_add_style(cont2, &style_glow, 0);

    /* V/I Data Container */
    lv_obj_t * header = lv_obj_create(cont2);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, GAUGE_SIZE + 60, LV_SIZE_CONTENT);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(header, 20, 0);
    //lv_obj_set_style_pad_bottom(header, 10, LV_PART_MAIN);
    //lv_obj_align_to(header, cont2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);    
    lv_obj_align(header, LV_ALIGN_BOTTOM_MID, 0, 10);

    voltage_display = create_display(header, lv_palette_main(LV_PALETTE_RED), false, 5);
    //lv_obj_align(voltage_display, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    lv_obj_t * label;

    label = lv_label_create(cont2);
    lv_obj_remove_style_all(label);
    lv_obj_move_background(label);
    lv_obj_align_to(label, voltage_display, LV_ALIGN_OUT_TOP_LEFT, 0, -20);    
    lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);    
    lv_label_set_text(label, "V");

    current_display = create_display(header, lv_palette_main(LV_PALETTE_RED), false, 5);
    //lv_obj_align(current_display, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    label = lv_label_create(cont2);
    lv_obj_remove_style_all(label);
    lv_obj_move_background(label);
    lv_obj_align_to(label, current_display, LV_ALIGN_OUT_TOP_RIGHT, 0, -20);    
    lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(label, "I");

    /* Main circular container */
    lv_obj_t * cont = lv_obj_create(cont2);
    lv_obj_set_size(cont, GAUGE_SIZE + 36, GAUGE_SIZE + 36);
    //lv_obj_center(cont);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 5, 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x818cf8), 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_scrollable(cont, false);

    // ---------- Dark track ----------
    arc_bg = lv_arc_create(cont);
    lv_obj_set_size(arc_bg, GAUGE_SIZE, GAUGE_SIZE);
    lv_obj_center(arc_bg);
    lv_arc_set_rotation(arc_bg, GAUGE_ROTATION);
    lv_arc_set_bg_angles(arc_bg, 0, ANGLE_RANGE);
    lv_arc_set_range(arc_bg, 0, GAUGE_MAX);
    lv_obj_remove_style(arc_bg, NULL, LV_PART_KNOB);
    lv_obj_set_clickable(arc_bg, false);

    lv_obj_set_style_arc_width(arc_bg, ARC_WIDTH + 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_bg, lv_color_hex(0x1e293b), LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arc_bg, true, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_bg, 0, LV_PART_INDICATOR);

    // ---------- Wide soft halo ----------
    arc_glow_outer = lv_arc_create(cont);
    lv_obj_set_size(arc_glow_outer, GAUGE_SIZE, GAUGE_SIZE);
    lv_obj_center(arc_glow_outer);
    lv_arc_set_rotation(arc_glow_outer, GAUGE_ROTATION);
    lv_arc_set_bg_angles(arc_glow_outer, 0, ANGLE_RANGE);
    lv_arc_set_range(arc_glow_outer, 0, GAUGE_MAX);
    lv_obj_remove_style(arc_glow_outer, NULL, LV_PART_KNOB);
    lv_obj_set_clickable(arc_glow_outer, false);

    lv_obj_set_style_arc_width(arc_glow_outer, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_glow_outer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_glow_outer, ARC_WIDTH + 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc_glow_outer, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_glow_outer, lv_color_hex(0x22d3ee), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc_glow_outer, LV_OPA_20, LV_PART_INDICATOR);

    // ---------- Tighter brighter glow ----------
    arc_glow_inner = lv_arc_create(cont);
    lv_obj_set_size(arc_glow_inner, GAUGE_SIZE, GAUGE_SIZE);
    lv_obj_center(arc_glow_inner);
    lv_arc_set_rotation(arc_glow_inner, GAUGE_ROTATION);
    lv_arc_set_bg_angles(arc_glow_inner, 0, ANGLE_RANGE);
    lv_arc_set_range(arc_glow_inner, 0, GAUGE_MAX);
    lv_obj_remove_style(arc_glow_inner, NULL, LV_PART_KNOB);
    lv_obj_set_clickable(arc_glow_inner, false);

    lv_obj_set_style_arc_width(arc_glow_inner, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_glow_inner, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_glow_inner, ARC_WIDTH + 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc_glow_inner, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_glow_inner, lv_color_hex(0x67e8f9), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc_glow_inner, LV_OPA_50, LV_PART_INDICATOR);

    // Wide soft halo
    //lv_obj_set_style_arc_color(arc_glow_outer, lv_color_hex(0x818cf8), LV_PART_INDICATOR); // soft indigo

    // Tighter brighter glow
    //lv_obj_set_style_arc_color(arc_glow_inner, lv_color_hex(0xa5b4fc), LV_PART_INDICATOR); // lighter indigo


    // ---------- Solid bright core ----------
    arc_indicator = lv_arc_create(cont);
    lv_obj_set_size(arc_indicator, GAUGE_SIZE, GAUGE_SIZE);
    lv_obj_center(arc_indicator);
    lv_arc_set_rotation(arc_indicator, GAUGE_ROTATION);
    lv_arc_set_bg_angles(arc_indicator, 0, ANGLE_RANGE);
    lv_arc_set_range(arc_indicator, 0, GAUGE_MAX);
    lv_obj_remove_style(arc_indicator, NULL, LV_PART_KNOB);
    lv_obj_set_clickable(arc_indicator, false);

    lv_obj_set_style_arc_width(arc_indicator, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc_indicator, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_indicator, ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc_indicator, true, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_indicator, lv_color_hex(0xa5f3fc), LV_PART_INDICATOR);

    // ---------- Scale ----------
    scale = lv_scale_create(cont);
    lv_obj_set_size(scale, GAUGE_SIZE - 8, GAUGE_SIZE - 8);
    lv_obj_center(scale);

    lv_obj_set_style_bg_opa(scale, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scale, 0, 0);

    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, 0, GAUGE_MAX);
    lv_scale_set_angle_range(scale, ANGLE_RANGE);
    lv_scale_set_rotation(scale, GAUGE_ROTATION);
    lv_scale_set_total_tick_count(scale, 49);
    lv_scale_set_major_tick_every(scale, 4);
    lv_scale_set_label_show(scale, true);

    lv_obj_set_style_length(scale, 13, LV_PART_INDICATOR);
    lv_obj_set_style_length(scale, 7,  LV_PART_ITEMS);
    lv_obj_set_style_line_width(scale, 3, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(scale, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_color(scale, lv_color_hex(0xe2e8f0), LV_PART_INDICATOR);
    lv_obj_set_style_line_color(scale, lv_color_hex(0x64748b), LV_PART_ITEMS);

    lv_obj_set_style_text_color(scale, lv_color_hex(0xf1f5f9), LV_PART_INDICATOR);
    lv_obj_set_style_text_font(scale, &lv_font_montserrat_14, LV_PART_INDICATOR);

    // Red zone
    lv_scale_section_t * red = lv_scale_add_section(scale);
    lv_scale_section_set_range(red, GAUGE_REDZONE, GAUGE_MAX);
    static lv_style_t style_red;
    lv_style_init(&style_red);
    lv_style_set_line_color(&style_red, lv_color_hex(0xef4444));
    lv_style_set_line_width(&style_red, 3);
    lv_scale_set_section_style_indicator(scale, red, &style_red);
    lv_scale_set_section_style_items(scale, red, &style_red);

    // ---------- Needle glow (thicker + transparent) ----------
    needle_glow = lv_line_create(scale);
    lv_obj_set_style_bg_opa(needle_glow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(needle_glow, 0, 0);
    lv_obj_set_style_pad_all(needle_glow, 0, 0);

    lv_obj_set_style_line_width(needle_glow, 11, 0);               // thicker
    lv_obj_set_style_line_color(needle_glow, lv_color_hex(0xfbbf24), 0);
    lv_obj_set_style_line_opa(needle_glow, LV_OPA_40, 0);          // soft
    lv_obj_set_style_line_rounded(needle_glow, true, 0);

    // ---------- Main needle (sharp) ----------
    needle_line = lv_line_create(scale);
    lv_obj_set_style_bg_opa(needle_line, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(needle_line, 0, 0);
    lv_obj_set_style_pad_all(needle_line, 0, 0);

    lv_obj_set_style_line_width(needle_line, 4, 0);
    lv_obj_set_style_line_color(needle_line, lv_color_hex(0xfde68a), 0); // slightly brighter
    lv_obj_set_style_line_rounded(needle_line, true, 0);

    // ---------- Glowing pivot (outer soft circle) ----------
    lv_obj_t * pivot_glow = lv_obj_create(cont);
    lv_obj_set_size(pivot_glow, 32, 32);                     // larger
    lv_obj_center(pivot_glow);
    lv_obj_set_style_radius(pivot_glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pivot_glow, lv_color_hex(0xfbbf24), 0);
    lv_obj_set_style_bg_opa(pivot_glow, LV_OPA_30, 0);       // soft glow
    lv_obj_set_style_border_width(pivot_glow, 0, 0);
    lv_obj_set_style_pad_all(pivot_glow, 0, 0);

    // ---------- Pivot ----------
    lv_obj_t * pivot = lv_obj_create(cont);
    lv_obj_set_size(pivot, 18, 18);
    lv_obj_center(pivot);
    lv_obj_set_style_radius(pivot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pivot, lv_color_hex(0xfbbf24), 0);
    lv_obj_set_style_border_width(pivot, 3, 0);
    lv_obj_set_style_border_color(pivot, lv_color_hex(0x0f172a), 0);

    // ---------- Digital value ----------
    label_value = lv_label_create(cont);
    lv_obj_remove_style_all(label_value);
    lv_obj_move_background(label_value);
    lv_obj_align(label_value, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_color(label_value, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_font(label_value, &lv_font_dseg7_32, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_label_set_text_fmt(label_value, "%03d", 0);

    lv_obj_t *digits_back = lv_label_create(cont);
    lv_obj_remove_style_all(digits_back);
    lv_obj_move_background(digits_back);
    lv_obj_align(digits_back, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_color(digits_back, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_text_font(digits_back, &lv_font_dseg7_32, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_opa(digits_back, LV_OPA_20, 0);
    lv_label_set_text(digits_back, "888");

    // Unit label
    lv_obj_t *label_unit = lv_label_create(cont);
    lv_obj_move_background(label_unit);
    lv_label_set_text(label_unit, "Watt");
    lv_obj_set_style_text_font(label_unit, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label_unit, lv_palette_main(LV_PALETTE_TEAL), 0);
    lv_obj_align(label_unit, LV_ALIGN_CENTER, 0, 36);

    digital_display = create_display(cont, lv_palette_main(LV_PALETTE_RED), false, 7);
    lv_obj_align(digital_display, LV_ALIGN_CENTER, 0, 120);

    gauge_set_power(150);
}
