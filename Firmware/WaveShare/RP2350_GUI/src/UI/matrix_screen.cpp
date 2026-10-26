#include "matrix_screen.h"
#include "pico/rand.h"
#include <stdlib.h>
#include <string.h>          // required for memset
#include <math.h>

#define BOUNCINGSAVER

#ifdef BOUNCINGSAVER

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 480
#define BOX_WIDTH     140
#define BOX_HEIGHT    70

typedef enum {
  dtV,
  dtI,
  dtP,
  dtE,
  dtT,
  dtS,
  //dtC,
  NumTypes
} DataType_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *label_title;
    lv_obj_t *label_val;
    lv_obj_t *label_display;
    int16_t x, y;
    int8_t vx, vy;
    uint16_t width, height;
    uint32_t last_value;
    uint32_t new_value;
    bool dirty;
    void (*set_value)(lv_obj_t * target, unsigned long value);
} bouncing_box_t;

typedef struct {
    bouncing_box_t boxes[BOX_COUNT];
    lv_timer_t *engine_timer;
    lv_obj_t *scr;
} telemetry_screen_data_t;

// Persistent, single-instance structures to eliminate dynamic heap allocation
static telemetry_screen_data_t s_telemetry_data = {0};
static bool s_telemetry_scr_initialized = false;

static volatile TMeasurementData current_telemetry = {12420, 1850, 22980, 0, 24750, 104500, 0};
static volatile bool telemetry_dirty = true;

static const lv_color_t colors[BOX_COUNT] = {
    lv_color_make(0x00, 0xE5, 0xFF),
    lv_color_make(0xFF, 0xB3, 0x00),
    lv_color_make(0xD5, 0x00, 0xF9),
    lv_color_make(0x00, 0xE6, 0x76),
    lv_color_make(0xFF, 0x00, 0x00),
    lv_color_make(0x00, 0x00, 0xFF)
};

static const char *titles[BOX_COUNT] = {"VOLTAGE", "CURRENT", "POWER", "ENERGY", "TEMPERATURE", "TIME"};
static const char *units[BOX_COUNT]  = {"V", "A", "W", "Wh", "°C", "s"};
static char label_buffer[BOX_COUNT][16] = {0};

extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t lv_font_montserrat_18;

// Helper AABB intersection check
static inline bool check_box_collision(bouncing_box_t *a, bouncing_box_t *b) {
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

// Box-to-Box Bounce & Separation Logic
static void resolve_box_collisions(bouncing_box_t boxes[BOX_COUNT]) {
    for (int i = 0; i < BOX_COUNT; i++) {
        for (int j = i + 1; j < BOX_COUNT; j++) {
            bouncing_box_t *b1 = &boxes[i];
            bouncing_box_t *b2 = &boxes[j];

            if (check_box_collision(b1, b2)) {
                // Calculate overlap depth along both axes
                int16_t overlap_x1 = (b1->x + b1->width) - b2->x;
                int16_t overlap_x2 = (b2->x + b2->width) - b1->x;
                int16_t overlap_x = (overlap_x1 < overlap_x2) ? overlap_x1 : overlap_x2;

                int16_t overlap_y1 = (b1->y + b1->height) - b2->y;
                int16_t overlap_y2 = (b2->y + b2->height) - b1->y;
                int16_t overlap_y = (overlap_y1 < overlap_y2) ? overlap_y1 : overlap_y2;

                // Resolve collision along the axis of minimum penetration
                if (overlap_x < overlap_y) {
                    // Bounce horizontally
                    int8_t temp_vx = b1->vx;
                    b1->vx = b2->vx;
                    b2->vx = temp_vx;

                    // Push boxes apart so they do not get stuck together
                    if (b1->x < b2->x) {
                        b1->x -= overlap_x / 2 + 1;
                        b2->x += overlap_x / 2 + 1;
                    } else {
                        b1->x += overlap_x / 2 + 1;
                        b2->x -= overlap_x / 2 + 1;
                    }
                } else {
                    // Bounce vertically
                    int8_t temp_vy = b1->vy;
                    b1->vy = b2->vy;
                    b2->vy = temp_vy;

                    // Push boxes apart
                    if (b1->y < b2->y) {
                        b1->y -= overlap_y / 2 + 1;
                        b2->y += overlap_y / 2 + 1;
                    } else {
                        b1->y += overlap_y / 2 + 1;
                        b2->y -= overlap_y / 2 + 1;
                    }
                }
            }
        }
    }
}

static void telemetry_engine_timer_cb(lv_timer_t * timer)
{
    int i;

    telemetry_screen_data_t * data =
        static_cast<telemetry_screen_data_t *>(lv_timer_get_user_data(timer));
    if (!data || !data->scr) return;

    // 1. Move all boxes according to their current velocity
    for ( i = 0; i < BOX_COUNT; i++ ) {
        bouncing_box_t *b = &data->boxes[i];
        if (!b->container) continue;

        b->x += b->vx;
        b->y += b->vy;
    }

    // 2. Resolve box-to-box collisions & rebound velocities
    resolve_box_collisions(data->boxes);

    // 3. Screen edge collision checks & boundary clamping
    for ( i = 0; i < BOX_COUNT; i++ ) {
        bouncing_box_t *b = &data->boxes[i];
        if (!b->container) continue;

        if (b->x <= 0) { 
            b->x = 0; 
            b->vx = abs(b->vx); 
        } else if (b->x + b->width >= SCREEN_WIDTH) {
            b->x = SCREEN_WIDTH - b->width;
            b->vx = -abs(b->vx);
        }

        if (b->y <= 0) { 
            b->y = 0; 
            b->vy = abs(b->vy); 
        } else if (b->y + b->height >= SCREEN_HEIGHT) {
            b->y = SCREEN_HEIGHT - b->height;
            b->vy = -abs(b->vy);
        }

        // Apply updated position to LVGL widget container
        lv_obj_set_pos(b->container, b->x, b->y);
    }

    //if (telemetry_dirty)
    {
        /*
        telemetry_dirty = false;

        uint32_t vals[BOX_COUNT] = {
            current_telemetry.V,
            current_telemetry.I,
            current_telemetry.P,
            current_telemetry.E,
            current_telemetry.T,
            current_telemetry.S
        };
        */

        for ( i = 0; i < BOX_COUNT; i++ ) {
            bouncing_box_t *b = &data->boxes[i];

            if (!b->dirty) continue;

            b->dirty = false;

            // Dismiss noise
            //if (abs((int32_t)(vals[i] - b->last_value)) <= 1) continue;
            //if (abs((int32_t)(b->new_value - b->last_value)) <= 1) continue;

            //b->last_value = vals[i];
            b->last_value = b->new_value;

            if ( (b->label_display != NULL) || (b->set_value != NULL) )
            {
                //b->set_value(b->label_display, vals[i]);
                b->set_value(b->label_display, b->last_value);
            }
        }
    }
}

static void init_telemetry_screen_once(void)
{
    memset(&s_telemetry_data, 0, sizeof(s_telemetry_data));

    lv_obj_t *telemetry_scr = lv_obj_create(NULL);
    if (!telemetry_scr) return;

    s_telemetry_data.scr = telemetry_scr;
    lv_obj_set_user_data(telemetry_scr, &s_telemetry_data);

    lv_obj_set_style_bg_color(telemetry_scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(telemetry_scr, LV_OPA_COVER, 0);

    const int16_t initial_positions[BOX_COUNT][2] = {{20, 30}, {260, 40}, {40, 260}, {250, 280}, {(SCREEN_WIDTH-BOX_WIDTH-4), (SCREEN_HEIGHT-BOX_HEIGHT-4)},{((SCREEN_WIDTH / 2) - (BOX_WIDTH / 2)), ((SCREEN_HEIGHT / 2) - (BOX_HEIGHT / 2))}};
    const int8_t  initial_velocities[BOX_COUNT][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {-2, -2}, {0, 0}};

    //uint32_t rand32;

    for (int i = 0; i < BOX_COUNT; i++) {

        DataType_t DT = (DataType_t)i;

        bouncing_box_t *b = &s_telemetry_data.boxes[i];
        b->x = initial_positions[i][0];
        b->y = initial_positions[i][1];

        //rand32 = get_rand_32();
        //b->x += (rand32 % 10);
        //rand32 = get_rand_32();
        //b->y += (rand32 % 10);

        b->vx = initial_velocities[i][0];
        b->vy = initial_velocities[i][1];
        b->width  = BOX_WIDTH;
        b->height = BOX_HEIGHT;
        b->last_value = -1;
        b->new_value = -1;

        b->container = NULL;
        b->label_title = NULL;
        b->label_val = NULL;
        b->label_display = NULL;
        b->dirty = true;

        b->container = lv_obj_create(telemetry_scr);
        if (!b->container) return;

        lv_obj_set_size(b->container, b->width, b->height);
        lv_obj_set_pos(b->container, b->x, b->y);
        lv_obj_set_scrollable(b->container, false);

        lv_obj_set_style_bg_color(b->container, lv_color_make(0x12, 0x18, 0x24), 0);
        lv_obj_set_style_bg_opa(b->container, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(b->container, colors[i], 0);
        lv_obj_set_style_border_width(b->container, 2, 0);
        lv_obj_set_style_radius(b->container, 8, 0);
        lv_obj_set_style_pad_all(b->container, 6, 0);

        b->label_title = lv_label_create(b->container);
        if (!b->label_title) return;
        lv_label_set_text(b->label_title, titles[i]);
        lv_obj_set_style_text_color(b->label_title, colors[i], 0);
        lv_obj_set_style_text_font(b->label_title, &lv_font_montserrat_12, 0);
        lv_obj_align(b->label_title, LV_ALIGN_TOP_MID, 0, 0);

        byte count = 5;
        if (i == 2) count = 6;
        if (i == 3) count = 6;
        if (i == 4) count = 3;
        if (i == 5) count = 6;
        b->label_display = create_display(b->container, lv_palette_main(LV_PALETTE_RED), false, count);
        if (!b->label_display) return;
        lv_obj_align(b->label_display, LV_ALIGN_BOTTOM_MID, 0, 0);

        if ( (i==4) || (i==5) )
        {
            b->set_value = SetDisplayV;
        }
        else
        {
            b->set_value = SetDisplaymV;
        }

    }

    s_telemetry_data.engine_timer = lv_timer_create(telemetry_engine_timer_cb, 40, &s_telemetry_data);
    if (s_telemetry_data.engine_timer) {
        lv_timer_pause(s_telemetry_data.engine_timer); // Paused until screen is navigated to
    }

    s_telemetry_scr_initialized = true;
}

void navigate_to_matrix_screen(void)
{
    // Create static layout once
    if (!s_telemetry_scr_initialized) {
        init_telemetry_screen_once();
    }

    if (!s_telemetry_data.scr) return;

    // Reset last_value on load to force full display refresh
    for (int i = 0; i < BOX_COUNT; i++) {
        s_telemetry_data.boxes[i].last_value = -1;
    }
    telemetry_dirty = true;

    // Resume execution of animation timer
    if (s_telemetry_data.engine_timer) {
        lv_timer_resume(s_telemetry_data.engine_timer);
    }

    // Load static telemetry screen (auto_del = false to prevent LVGL from freeing it)
    lv_screen_load_anim(s_telemetry_data.scr, LV_SCR_LOAD_ANIM_NONE, 100, 0, false);
}

void pause_matrix_screen(void)
{
    if (s_telemetry_data.engine_timer) {
        lv_timer_pause(s_telemetry_data.engine_timer);
    }
}

void telemetry_screensaver_update_i2c(const PMeasurementData data)
{
    if (data) {
        telemetry_screensaver_update_V(data->V);
        telemetry_screensaver_update_I(data->I);
        telemetry_screensaver_update_P(data->P);
        telemetry_screensaver_update_E(data->E);
        telemetry_screensaver_update_box5(data->T);
        telemetry_screensaver_update_box6(data->S);
    }
}

void telemetry_screensaver_update_V(const uint16_t data)
{
    static uint16_t oldData = -1;
    if (oldData != data)
    {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtV].new_value = data;
            s_telemetry_data.boxes[dtV].dirty = true;
        }
        //current_telemetry.V = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

void telemetry_screensaver_update_I(const uint16_t data)
{
    static uint16_t oldData = -1;
    if (oldData != data) {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtI].new_value = data;
            s_telemetry_data.boxes[dtI].dirty = true;
        }
        //current_telemetry.I = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

void telemetry_screensaver_update_P(const uint32_t data)
{
    static uint32_t oldData = -1;
    if (oldData != data) {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtP].new_value = data;
            s_telemetry_data.boxes[dtP].dirty = true;
        }
        //current_telemetry.P = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

void telemetry_screensaver_update_E(const uint32_t data)
{
    static uint32_t oldData = -1;
    if (oldData != data) {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtE].new_value = data;
            s_telemetry_data.boxes[dtE].dirty = true;
        }
        //current_telemetry.E = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

void telemetry_screensaver_update_box5(const uint32_t data)
{
    static uint32_t oldData = -1;
    if (oldData != data) {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtT].new_value = data;
            s_telemetry_data.boxes[dtT].dirty = true;
        }
        //current_telemetry.T = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

void telemetry_screensaver_update_box6(const uint32_t data)
{
    static uint32_t oldData = -1;
    if (oldData != data) {
        if (s_telemetry_scr_initialized)
        {
            s_telemetry_data.boxes[dtS].new_value = data;
            s_telemetry_data.boxes[dtS].dirty = true;
        }
        //current_telemetry.S = data;
        //telemetry_dirty = true;
        oldData = data;
    }
}

#endif