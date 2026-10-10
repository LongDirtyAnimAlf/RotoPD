#include "Screenbase.h"

lv_obj_t * screensettings = NULL;

#include "lvgl.h"

// Declare CANopen logo image asset
LV_IMG_DECLARE(canopen_logo_img);

static lv_obj_t * spinbox_node1;
static lv_obj_t * spinbox_node2;

/* Helper callback for Spinbox decrement button */
static void spinbox_decrement_cb(lv_event_t * e)
{
    lv_obj_t * sb = (lv_obj_t *)lv_event_get_user_data(e);
    lv_spinbox_decrement(sb);
}

/* Helper callback for Spinbox increment button */
static void spinbox_increment_cb(lv_event_t * e)
{
    lv_obj_t * sb = (lv_obj_t *)lv_event_get_user_data(e);
    lv_spinbox_increment(sb);
}

/* Helper to construct a node address spinbox with +/- buttons */
static lv_obj_t * create_node_address_spinbox(lv_obj_t * parent, int initial_val, lv_obj_t ** spinbox_ptr)
{
    lv_obj_t * container = lv_obj_create(parent);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Minus Button */
    lv_obj_t * btn_decrement = lv_button_create(container);
    lv_obj_set_size(btn_decrement, 36, 36);
    lv_obj_set_style_bg_color(btn_decrement, lv_color_hex(0x3B3B52), LV_PART_MAIN);
    lv_obj_t * lbl_minus = lv_label_create(btn_decrement);
    lv_label_set_text(lbl_minus, LV_SYMBOL_MINUS);
    lv_obj_center(lbl_minus);

    /* Spinbox widget (restricted to range 1-8) */
    lv_obj_t * sb = lv_spinbox_create(container);
    lv_spinbox_set_range(sb, 1, 8);
    lv_spinbox_set_digit_count(sb, 1);
    lv_spinbox_set_value(sb, initial_val);
    lv_obj_set_width(sb, 40);
    lv_obj_set_style_text_align(sb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sb, lv_color_hex(0x1E1E2E), LV_PART_MAIN);
    lv_obj_set_style_text_color(sb, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(sb, lv_color_hex(0x00A86B), LV_PART_MAIN);

    /* Plus Button */
    lv_obj_t * btn_increment = lv_button_create(container);
    lv_obj_set_size(btn_increment, 36, 36);
    lv_obj_set_style_bg_color(btn_increment, lv_color_hex(0x3B3B52), LV_PART_MAIN);
    lv_obj_t * lbl_plus = lv_label_create(btn_increment);
    lv_label_set_text(lbl_plus, LV_SYMBOL_PLUS);
    lv_obj_center(lbl_plus);

    /* Attach event listeners */
    lv_obj_add_event_cb(btn_decrement, spinbox_decrement_cb, LV_EVENT_CLICKED, sb);
    lv_obj_add_event_cb(btn_increment, spinbox_increment_cb, LV_EVENT_CLICKED, sb);

    if(spinbox_ptr) *spinbox_ptr = sb;

    return container;
}

lv_obj_t * create_canopen_logo_widget(lv_obj_t * parent)
{
    /* Container badge representing the CANopen badge */
    lv_obj_t * logo_container = lv_obj_create(parent);
    lv_obj_remove_style_all(logo_container);
    lv_obj_set_size(logo_container, 100, 30);
    
    // CiA Corporate CANopen Blue (#00529C)
    lv_obj_set_style_bg_color(logo_container, lv_color_hex(0x00529C), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(logo_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(logo_container, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(logo_container, 8, LV_PART_MAIN);
    
    // Internal Flex Layout for exact text alignment
    lv_obj_set_layout(logo_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(logo_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(logo_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* "CAN" - Bold white label */
    lv_obj_t * label_can = lv_label_create(logo_container);
    lv_label_set_text(label_can, "CAN");
    lv_obj_set_style_text_color(label_can, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_can, &lv_font_montserrat_14, LV_PART_MAIN);

    /* "open" - Lowercase cyan accent label */
    lv_obj_t * label_open = lv_label_create(logo_container);
    lv_label_set_text(label_open, "open");
    lv_obj_set_style_text_color(label_open, lv_color_hex(0x00D0FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_open, &lv_font_montserrat_14, LV_PART_MAIN);

    return logo_container;
}

void create_canopen_setup_screen(lv_obj_t * parent)
{
    /* 1. Main Screen Setup */
    lv_obj_t * scr = parent;
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1E1E2E), LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr, 12, LV_PART_MAIN);
    lv_obj_set_layout(scr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* 2. Header Container (Title + CANopen Logo) */
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_bottom(header, 10, LV_PART_MAIN);

    // Title
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "CANopen Network Setup");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, LV_PART_MAIN);



    // CANopen Logo / Badge
    lv_obj_t * logo = create_canopen_logo_widget(header);
    //lv_obj_t * logo = lv_image_create(header);
    // lv_image_set_src(logo, &canopen_logo_img); // Uncomment when asset is linked
    //lv_obj_set_size(logo, 80, 28);
    //lv_obj_set_style_bg_color(logo, lv_color_hex(0x0055A5), LV_PART_MAIN);
    //lv_obj_set_style_bg_opa(logo, LV_OPA_COVER, LV_PART_MAIN);
    //lv_obj_set_style_radius(logo, 4, LV_PART_MAIN);
    //lv_obj_t * logo_txt = lv_label_create(logo);
    //lv_label_set_text(logo_txt, "CANopen");
    //lv_obj_center(logo_txt);
    //lv_obj_set_style_text_color(logo_txt, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    /* 3. Settings Card Container */
    lv_obj_t * card = lv_obj_create(scr);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_flex_grow(card, 1);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x2A2A3C), LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(0x3B3B52), LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 16, LV_PART_MAIN);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(card, 12, LV_PART_MAIN);

    /* 4. Three Switch Toggles */
    const char * toggle_labels[3] = {
        "Enable Bus Termination (120 Ω)",
        "Auto Baudrate Detection",
        "Heartbeat Monitoring"
    };

    for(int i = 0; i < 3; i++) {
        lv_obj_t * row = lv_obj_create(card);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t * lbl = lv_label_create(row);
        lv_label_set_text(lbl, toggle_labels[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE0E0E0), LV_PART_MAIN);

        lv_obj_t * sw = lv_switch_create(row);
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x00A86B), LV_PART_INDICATOR | LV_STATE_CHECKED);
        if(i == 0) lv_obj_add_state(sw, LV_STATE_CHECKED);
    }

    /* Divider */
    lv_obj_t * line = lv_obj_create(card);
    lv_obj_set_size(line, LV_PCT(100), 1);
    lv_obj_set_style_bg_color(line, lv_color_hex(0x3B3B52), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, LV_PART_MAIN);

    /* 5. Two Node Address Spinboxes (Bounded strictly 1 to 8) */
    const char * node_labels[2] = {"Master Node ID (1-8):", "Sub-Node ID (1-8):"};
    lv_obj_t ** spinboxes[2] = {&spinbox_node1, &spinbox_node2};
    int initial_values[2] = {1, 2};

    for(int i = 0; i < 2; i++) {
        lv_obj_t * row = lv_obj_create(card);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t * lbl = lv_label_create(row);
        lv_label_set_text(lbl, node_labels[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE0E0E0), LV_PART_MAIN);

        create_node_address_spinbox(row, initial_values[i], spinboxes[i]);
    }
}

static void Setup_Screen(lv_obj_t * cont)
{
    if (lv_obj_get_child_count(cont) == 0)
    {
        create_canopen_setup_screen(cont);
    }
}

void Setup_ScreenSettings(byte index, bool show)
{
    lv_obj_t * obj = NULL;

    if (show)
    {
        obj = GetInfoObject();
        if (obj) lv_label_set_text(obj, "Settings");

        obj = GetButtonLabelObject();
        if (obj) lv_label_set_text(obj, "Logger");
    }

    SetContentObject(screensettings, show);
    if (screensettings) Setup_Screen(screensettings);

}
