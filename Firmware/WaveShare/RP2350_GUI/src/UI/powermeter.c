#include "lvgl.h"
#include "Screenbase.h"

// ============== Configuration ==============
#define GAUGE_MAX           240
#define GAUGE_REDZONE       140
#define ANGLE_RANGE         270
#define GAUGE_ROTATION      135
#define GAUGE_SIZE          340
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
static lv_obj_t * needle_tip;          // bright red tip
static lv_obj_t * label_value;
static lv_obj_t * label_unit;
static lv_obj_t * digital_display;

// ============== Helper: place the red tip ==============
static void update_needle_tip(int32_t power)
{
    // LVGL angle: 0° = 3 o'clock, positive = clockwise
    float angle_deg = (float)GAUGE_ROTATION + 
                      ((float)power * ANGLE_RANGE) / (float)GAUGE_MAX;

    float rad = angle_deg * (M_PI / 180.0f);

    // Center of the scale
    int32_t cx = (GAUGE_SIZE - 8) / 2;   // scale size is GAUGE_SIZE-8
    int32_t cy = (GAUGE_SIZE - 8) / 2;

    // Correct formula for LVGL coordinate system (Y positive downward)
    int32_t tip_x = cx + (int32_t)(cosf(rad) * (NEEDLE_LENGTH + 4));
    int32_t tip_y = cy + (int32_t)(sinf(rad) * (NEEDLE_LENGTH + 4));

    lv_obj_set_pos(needle_tip,
                   tip_x - TIP_RADIUS - 1,
                   tip_y - TIP_RADIUS- 1);
}

// ============== Update ==============
void gauge_set_value(int32_t power)
{
    if (power < 0)          power = 0;
    if (power > GAUGE_MAX)  power = GAUGE_MAX;

    lv_arc_set_value(arc_indicator, power);
    lv_arc_set_value(arc_glow_outer, power);
    lv_arc_set_value(arc_glow_inner, power);

    // Both needle layers must be updated
    lv_scale_set_line_needle_value(scale, needle_glow, NEEDLE_LENGTH + 4, power);
    lv_scale_set_line_needle_value(scale, needle_line, NEEDLE_LENGTH, power);

    update_needle_tip(power);          // move the red tip

    lv_label_set_text_fmt(label_value, "%d", (int)power);
}

// ============== Create ==============
void create_3d_gauge(lv_obj_t * parent)
{
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0a0e17), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    lv_obj_remove_flag(parent, LV_OBJ_FLAG_SCROLLABLE);    

    static lv_style_t style2;
    lv_style_init(&style2);
    //lv_style_set_radius(&style, LV_RADIUS_CIRCLE); 
    lv_style_set_bg_opa(&style2, LV_OPA_COVER);
    lv_style_set_bg_color(&style2, lv_color_hex(0x0f172a));
    lv_style_set_shadow_width(&style2, 40);
    lv_style_set_shadow_spread(&style2, 5);
    lv_style_set_shadow_color(&style2, lv_palette_main(LV_PALETTE_CYAN));
    lv_style_set_shadow_offset_x(&style2, 0);
    lv_style_set_shadow_offset_y(&style2, 0);
    // Requires LVGL v9.5 or newer
    //lv_style_set_drop_shadow_color(&style, lv_palette_main(LV_PALETTE_RED));
    //lv_style_set_drop_shadow_radius(&style, 16);
    //lv_style_set_drop_shadow_opa(&style, LV_OPA_COVER);

    lv_obj_t * cont2 = lv_obj_create(parent);
    lv_obj_set_size(cont2, GAUGE_SIZE + 50, GAUGE_SIZE + 50);
    lv_obj_remove_flag(cont2, LV_OBJ_FLAG_SCROLLABLE);    
    lv_obj_center(cont2);
    lv_obj_add_style(cont2, &style2, 0);

    lv_obj_t * cont = lv_obj_create(cont2);
    lv_obj_set_size(cont, GAUGE_SIZE + 36, GAUGE_SIZE + 36);
    lv_obj_center(cont);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x0f172a), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 5, 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x818cf8), 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);


    // ---------- Dark track ----------
    arc_bg = lv_arc_create(cont);
    lv_obj_set_size(arc_bg, GAUGE_SIZE, GAUGE_SIZE);
    lv_obj_center(arc_bg);
    lv_arc_set_rotation(arc_bg, GAUGE_ROTATION);
    lv_arc_set_bg_angles(arc_bg, 0, ANGLE_RANGE);
    lv_arc_set_range(arc_bg, 0, GAUGE_MAX);
    lv_obj_remove_style(arc_bg, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc_bg, LV_OBJ_FLAG_CLICKABLE);

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
    lv_obj_remove_flag(arc_glow_outer, LV_OBJ_FLAG_CLICKABLE);

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
    lv_obj_remove_flag(arc_glow_inner, LV_OBJ_FLAG_CLICKABLE);

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
    lv_obj_remove_flag(arc_indicator, LV_OBJ_FLAG_CLICKABLE);

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
    lv_scale_section_set_style(red, LV_PART_INDICATOR, &style_red);
    lv_scale_section_set_style(red, LV_PART_ITEMS, &style_red);

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

// ---------- Bright red tip ----------
    needle_tip = lv_obj_create(scale);
    lv_obj_set_size(needle_tip, TIP_RADIUS * 2, TIP_RADIUS * 2);
    lv_obj_set_style_radius(needle_tip, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(needle_tip, lv_color_hex(0xff2020), 0);   // bright red
    lv_obj_set_style_bg_opa(needle_tip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(needle_tip, 0, 0);
    lv_obj_set_style_pad_all(needle_tip, 0, 0);
    // optional tiny glow around the tip
    lv_obj_set_style_shadow_width(needle_tip, 8, 0);
    lv_obj_set_style_shadow_color(needle_tip, lv_color_hex(0xff2020), 0);
    lv_obj_set_style_shadow_opa(needle_tip, LV_OPA_50, 0);


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

    // ---------- Digital ----------
    label_value = lv_label_create(cont);
    lv_label_set_text(label_value, "0");
    lv_obj_set_style_text_font(label_value, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(label_value, lv_color_hex(0xf8fafc), 0);
    lv_obj_align(label_value, LV_ALIGN_CENTER, 0, -42);

    label_unit = lv_label_create(cont);
    lv_label_set_text(label_unit, "Watt");
    lv_obj_set_style_text_font(label_unit, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label_unit, lv_color_hex(0x94a3b8), 0);
    lv_obj_align(label_unit, LV_ALIGN_CENTER, 0, 36);

    digital_display = create_display(cont, lv_palette_main(LV_PALETTE_RED), false, 7);    
    lv_obj_align(digital_display, LV_ALIGN_CENTER, 0, 136);

    gauge_set_value(150);
}
