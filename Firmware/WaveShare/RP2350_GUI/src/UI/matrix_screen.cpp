#include "matrix_screen.h"
#include <stdlib.h>
#include <string.h>          // ← required for memset
#include <math.h>

#define NEWSAVER

#ifdef NEWSAVER

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 480
#define BOX_WIDTH     140
#define BOX_HEIGHT    70

typedef struct {
    lv_obj_t *container;
    lv_obj_t *label_title;
    lv_obj_t *label_val;
    lv_obj_t *label_display;
    int16_t x, y;
    int8_t vx, vy;
    uint16_t width, height;
    uint32_t last_value;
} bouncing_box_t;

typedef struct {
    bouncing_box_t boxes[4];
    lv_timer_t *engine_timer;
    lv_obj_t *scr;
} telemetry_screen_data_t;

static TMeasurementData current_telemetry = {12420, 1850, 22980, 230, 24750, 104500, 0};
static bool telemetry_dirty = true;

static const lv_color_t colors[4] = {
    lv_color_make(0x00, 0xE5, 0xFF),
    lv_color_make(0xFF, 0xB3, 0x00),
    lv_color_make(0xD5, 0x00, 0xF9),
    lv_color_make(0x00, 0xE6, 0x76)
};

static const char *titles[4] = {"VOLTAGE", "CURRENT", "POWER", "ENERGY"};
static const char *units[4]  = {"V", "A", "W", "Wh"};

static char label_buffer[4][16] = {0};

extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t lv_font_montserrat_18;

static void telemetry_engine_timer_cb(lv_timer_t * timer)
{
    int i;

    telemetry_screen_data_t * data =
        static_cast<telemetry_screen_data_t *>(lv_timer_get_user_data(timer));
    if (!data) return;

    lv_obj_t * scr = data->scr;
    if (!scr) return;   // do NOT call lv_obj_is_valid yet

    for ( i = 0; i < 4; i++ ) {
        bouncing_box_t *b = &data->boxes[i];
        if (!b->container) continue;

        b->x += b->vx;
        b->y += b->vy;

        if (b->x <= 0) { b->x = 0; b->vx = -b->vx; }
        else if (b->x + b->width >= SCREEN_WIDTH) {
            b->x = SCREEN_WIDTH - b->width;
            b->vx = -b->vx;
        }
        if (b->y <= 0) { b->y = 0; b->vy = -b->vy; }
        else if (b->y + b->height >= SCREEN_HEIGHT) {
            b->y = SCREEN_HEIGHT - b->height;
            b->vy = -b->vy;
        }

        lv_obj_set_pos(b->container, b->x, b->y);
    }

    if (telemetry_dirty) {

        telemetry_dirty = false;

        uint32_t vals[4] = {
            current_telemetry.V,
            current_telemetry.I,
            current_telemetry.P,
            current_telemetry.E
        };

        for ( i = 0; i < 4; i++ ) {
            bouncing_box_t *b = &data->boxes[i];

            // Dismiss noise
            if (abs((int32_t)(vals[i] - b->last_value)) <= 1) continue;

            b->last_value = vals[i];

            if (b->label_display != NULL) SetDisplaymV(b->label_display, vals[i]);

            /*
            if (b->label_val != NULL)            
            {
                if ((i == 2) || (i == 3))
                    snprintf(label_buffer[i], sizeof(label_buffer[i]),  "%.0f %s", (double)vals[i], units[i]);
                else
                    snprintf(label_buffer[i], sizeof(label_buffer[i]),  "%.3f %s", (double)vals[i], units[i]);

                //lv_obj_invalidate(b->label_val);                
            }
            */
        }
    }
}

static void telemetry_screen_delete_cb(lv_event_t *e)
{
    lv_obj_t *scr = lv_event_get_target_obj(e);
    telemetry_screen_data_t *data =
        static_cast<telemetry_screen_data_t *>(lv_obj_get_user_data(scr));
    if (!data) return;

    lv_obj_set_user_data(scr, NULL);
    data->scr = NULL;

    if (data->engine_timer) {
        lv_timer_delete(data->engine_timer);
        data->engine_timer = NULL;
    }
    free(data);
}

void navigate_to_matrix_screen(void)
{
    telemetry_screen_data_t *data =
        (telemetry_screen_data_t *)malloc(sizeof(telemetry_screen_data_t));
    if (!data) return;

    memset(data, 0, sizeof(*data));

    lv_obj_t *telemetry_scr = lv_obj_create(NULL);
    if (!telemetry_scr) {
        free(data);
        return;
    }

    data->scr = telemetry_scr;
    lv_obj_set_user_data(telemetry_scr, data);

    lv_obj_set_style_bg_color(telemetry_scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(telemetry_scr, LV_OPA_COVER, 0);

    const int16_t initial_positions[4][2] = {{20, 30}, {260, 40}, {40, 260}, {250, 280}};
    //const int8_t  initial_velocities[4][2] = {{3, 2}, {-2, 3}, {3, -3}, {-3, -2}};
    const int8_t  initial_velocities[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

    for (int i = 0; i < 4; i++) {
        bouncing_box_t *b = &data->boxes[i];
        b->x = initial_positions[i][0];
        b->y = initial_positions[i][1];
        b->vx = initial_velocities[i][0];
        b->vy = initial_velocities[i][1];
        b->width  = BOX_WIDTH;
        b->height = BOX_HEIGHT;
        b->last_value = -1;

        b->container = NULL;
        b->label_title = NULL;
        b->label_val = NULL;
        b->label_display = NULL;


        b->container = lv_obj_create(telemetry_scr);
        if (!b->container) {               // ← critical
            // clean up what we already created and abort
            lv_obj_delete(telemetry_scr);  // will also free data via delete_cb
            return;
        }

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
        if (!b->label_title) {
            lv_obj_delete(telemetry_scr);
            return;
        }
        lv_label_set_text(b->label_title, titles[i]);
        lv_obj_set_style_text_color(b->label_title, colors[i], 0);
        lv_obj_set_style_text_font(b->label_title, &lv_font_montserrat_12, 0);
        lv_obj_align(b->label_title, LV_ALIGN_TOP_MID, 0, 0);

        /*
        b->label_val = lv_label_create(b->container);
        if (!b->label_val) {
            lv_obj_delete(telemetry_scr);
            return;
        }

        //lv_label_set_text(b->label_val, "--.--");
        lv_obj_set_style_text_color(b->label_val, lv_color_white(), 0);
        lv_obj_set_style_text_font(b->label_val, &lv_font_montserrat_18, 0);
        lv_obj_align(b->label_val, LV_ALIGN_BOTTOM_MID, 0, 0);
        snprintf(label_buffer[i], sizeof(label_buffer[i]),  "%s", "--.--");
        lv_label_set_text_static(b->label_val, label_buffer[i]); 
        lv_obj_set_hidden(b->label_val, true);        
        */

        byte count = 5;
        if (i == 2) count = 6;
        if (i == 3) count = 6;
        b->label_display = create_display(b->container, lv_palette_main(LV_PALETTE_RED), false, count);
        if (!b->label_display) {
            lv_obj_delete(telemetry_scr);
            return;
        }
        lv_obj_align(b->label_display, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

    telemetry_dirty = true;

    lv_obj_add_event_cb(telemetry_scr, telemetry_screen_delete_cb, LV_EVENT_DELETE, NULL);

    data->engine_timer = lv_timer_create(telemetry_engine_timer_cb, 100, data);
    if (!data->engine_timer) {
        lv_obj_delete(telemetry_scr);
        return;
    }

    lv_screen_load_anim(telemetry_scr, LV_SCR_LOAD_ANIM_NONE, 100, 0, false);
}

void telemetry_screensaver_update_i2c(const PMeasurementData data)
{
    if (data) {
        current_telemetry = *data;
        telemetry_dirty = true;
    }
}

#endif

#ifdef OLDSAVER

#define SCREEN_WIDTH   480
#define SCREEN_HEIGHT  480
#define MAX_STREAMS    18   // Reduced slightly to fit RP2350 memory-bandwidth sweet spot
#define MAX_TAIL_LEN   14

typedef struct {
    int16_t x;
    float y;
    float speed;
    uint8_t tail_len;
    char chars[MAX_TAIL_LEN]; // Cached characters to eliminate rand() in draw callback
} matrix_stream_t;

typedef struct {
    matrix_stream_t streams[MAX_STREAMS];
    lv_timer_t * engine_timer;
} matrix_screen_data_t;

static const char matrix_source_chars[] = "DONALFREDO";
#define SOURCE_CHAR_COUNT (sizeof(matrix_source_chars) - 1)

static inline char get_random_matrix_char(void) {
    static uint8_t index = 0;
    //return matrix_source_chars[rand() % SOURCE_CHAR_COUNT];
    return matrix_source_chars[(255-index++) % SOURCE_CHAR_COUNT];
}

// 1. Optimized Draw Callback
static void matrix_draw_cb(lv_event_t * e) {
    lv_obj_t * scr = lv_event_get_target_obj(e);
    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    lv_layer_t * layer = lv_event_get_layer(e);
    
    if (!data || !layer) return;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = LV_FONT_DEFAULT;

    // Cache common color descriptors
    const lv_color_t head_color = lv_color_hex(0xD0FFD0);
    const lv_color_t tail_color = lv_color_hex(0x00C800);

    for (int i = 0; i < MAX_STREAMS; i++) {
        matrix_stream_t *stream = &data->streams[i];
        int16_t current_y = (int16_t)stream->y;
        
        for (int j = 0; j < stream->tail_len; j++) {
            int16_t char_y = current_y - (j * 14);

            // Fast clipping check
            if (char_y < -14 || char_y > SCREEN_HEIGHT) continue;

            // Discrete Opacity Tiers (Fast-path rendering)
            if (j == 0) {
                label_dsc.color = head_color;
                label_dsc.opa = LV_OPA_COVER;
            } else if (j < 3) {
                label_dsc.color = tail_color;
                label_dsc.opa = LV_OPA_80;
            } else {
                label_dsc.color = tail_color;
                label_dsc.opa = LV_OPA_40;
            }

            lv_point_t point = { stream->x, char_y };
            
            // Draw cached character code point without executing rand()
            lv_draw_character(layer, &label_dsc, &point, (uint32_t)stream->chars[j]);
        }
    }
}

// 2. Animation Engine Timer (50ms / 20 FPS)
static void matrix_engine_timer_cb(lv_timer_t * timer) {
    lv_obj_t * scr = (lv_obj_t*)(lv_timer_get_user_data(timer));
    if (!scr) return;

    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    if (!data) return;

    for (int i = 0; i < MAX_STREAMS; i++) {
        matrix_stream_t *stream = &data->streams[i];
        stream->y += stream->speed;

        // Reset stream when it leaves the bottom of the screen
        if (stream->y - (stream->tail_len * 14) > SCREEN_HEIGHT) {
            stream->y = -15.0f;
            stream->speed = 3.0f + ((rand() % 40) / 10.0f);
            
            // Refresh character array on stream reset
            for (int k = 0; k < stream->tail_len; k++) {
                stream->chars[k] = get_random_matrix_char();
            }
        }
    }

    lv_obj_invalidate(scr);
}

// 3. Cleanup Routine
static void matrix_screen_delete_cb(lv_event_t * e) {
    lv_obj_t * scr = lv_event_get_target_obj(e);
    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    
    if (data) {
        if (data->engine_timer) {
            lv_timer_delete(data->engine_timer);
        }
        free(data);
    }
}

// 4. Main Entry Point
void navigate_to_matrix_screen(void) {
    matrix_screen_data_t * data = (matrix_screen_data_t*)(malloc(sizeof(matrix_screen_data_t)));
    if (!data) return;

    int16_t spacing = SCREEN_WIDTH / MAX_STREAMS;
    for (int i = 0; i < MAX_STREAMS; i++) {
        data->streams[i].x = i * spacing + (rand() % 4);
        data->streams[i].y = (float)(-(rand() % SCREEN_HEIGHT));
        data->streams[i].speed = 3.0f + ((rand() % 40) / 10.0f);
        data->streams[i].tail_len = 6 + (rand() % 6);
        
        for (int k = 0; k < MAX_TAIL_LEN; k++) {
            data->streams[i].chars[k] = get_random_matrix_char();
        }
    }

    lv_obj_t * matrix_scr = lv_obj_create(NULL); 
    lv_obj_set_user_data(matrix_scr, data);

    lv_obj_set_style_bg_color(matrix_scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(matrix_scr, LV_OPA_COVER, 0);

    lv_obj_add_event_cb(matrix_scr, matrix_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(matrix_scr, matrix_screen_delete_cb, LV_EVENT_DELETE, NULL);

    // Set frame tick interval to 50ms (20 FPS) to drastically cut RP2350 draw overhead
    data->engine_timer = lv_timer_create(matrix_engine_timer_cb, 20, matrix_scr);

    lv_screen_load_anim(matrix_scr, LV_SCR_LOAD_ANIM_NONE, 100, 0, false);
}
#endif
