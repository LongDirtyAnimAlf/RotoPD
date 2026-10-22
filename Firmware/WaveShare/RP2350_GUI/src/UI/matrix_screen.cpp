#include "matrix_screen.h"
#include <stdlib.h>

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
    return matrix_source_chars[rand() % SOURCE_CHAR_COUNT];
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
    data->engine_timer = lv_timer_create(matrix_engine_timer_cb, 50, matrix_scr);

    lv_screen_load_anim(matrix_scr, LV_SCR_LOAD_ANIM_NONE, 100, 0, false);
}

#ifdef YOLOMAAKMEGEK 

#include "matrix_screen.h"
#include <stdlib.h>

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 480
#define MAX_STREAMS   24   // Number of vertical streams running simultaneously

typedef struct {
    int16_t x;
    float y;
    float speed;
    uint8_t tail_len;
} matrix_stream_t;

typedef struct {
    matrix_stream_t streams[MAX_STREAMS];
    lv_timer_t * engine_timer;
} matrix_screen_data_t;

// Explicit source character pool array
static const char matrix_source_chars[] = "DONALFREDO";
#define SOURCE_CHAR_COUNT (sizeof(matrix_source_chars) - 1)

// Get a random printable Unicode/ASCII code point ('!' to 'Z')
static uint32_t get_random_matrix_char(void) {
    //return (uint32_t)(33 + (rand() % 58)); 

    // Generates a random index strictly bounded by the length of "DONALFREDO"
    uint32_t random_index = rand() % SOURCE_CHAR_COUNT;
    
    // Cast the specific character byte cleanly to a Unicode/ASCII code point
    return (uint32_t)(matrix_source_chars[random_index]); 
}

// 1. Blazing Fast Public Character Render Callback
static void matrix_draw_cb(lv_event_t * e) {
    lv_obj_t * scr = lv_event_get_target_obj(e);
    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    lv_layer_t * layer = lv_event_get_layer(e);
    
    if(!data || !layer) return;

    // Use standard label descriptor for styling parameters
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = LV_FONT_DEFAULT; // Pulls from active font configuration

    for(int i = 0; i < MAX_STREAMS; i++) {
        int16_t current_y = (int16_t)data->streams[i].y;
        
        for(int j = 0; j < data->streams[i].tail_len; j++) {
            int16_t char_y = current_y - (j * 13); // Quick line spacing math

            // Bounds Clipping check
            if(char_y < 0 || char_y > SCREEN_HEIGHT - 14) continue;

            // Apply light/dark color mappings natively to the character
            if(j == 0) {
                label_dsc.color = lv_color_hex(0xD0FFD0); // Bright white-green head droplet
                label_dsc.opa = LV_OPA_COVER;
            } else {
                label_dsc.color = lv_color_hex(0x00C800); // Standard green tail
                label_dsc.opa = (lv_opa_t)(LV_OPA_COVER - ((j * LV_OPA_COVER) / data->streams[i].tail_len));
            }

            // Define exact coordinate point for the top-left of the single letter
            lv_point_t point;
            point.x = data->streams[i].x;
            point.y = char_y;

            // FIX: Use the native public character rendering API for LVGL 9
            uint32_t unicode_letter = get_random_matrix_char();
            lv_draw_character(layer, &label_dsc, &point, unicode_letter);
        }
    }
}

// 2. Animation Engine Timer
static void matrix_engine_timer_cb(lv_timer_t * timer) {
    lv_obj_t * scr = (lv_obj_t*)(lv_timer_get_user_data(timer));
    if(!scr) return;

    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    if(!data) return;

    for(int i = 0; i < MAX_STREAMS; i++) {
        data->streams[i].y += data->streams[i].speed;

        if(data->streams[i].y - (data->streams[i].tail_len * 13) > SCREEN_HEIGHT) {
            data->streams[i].y = -15.0f;
            data->streams[i].speed = 3.5f + ((rand() % 50) / 10.0f);
        }
    }

    // Invalidate screen to trigger the render loop redraw hook
    lv_obj_invalidate(scr);
}

// 3. Cleanup Routine
static void matrix_screen_delete_cb(lv_event_t * e) {
    lv_obj_t * scr = lv_event_get_target_obj(e);
    matrix_screen_data_t * data = (matrix_screen_data_t*)(lv_obj_get_user_data(scr));
    
    if(data) {
        if(data->engine_timer) {
            lv_timer_delete(data->engine_timer);
        }
        free(data);
    }
}

// 4. Main Matrix Screensaver Initialization Entry point
void navigate_to_matrix_screen(void) {
    matrix_screen_data_t * data = (matrix_screen_data_t*)(malloc(sizeof(matrix_screen_data_t)));
    if(!data) return;

    int16_t spacing = SCREEN_WIDTH / MAX_STREAMS;
    for(int i = 0; i < MAX_STREAMS; i++) {
        data->streams[i].x = i * spacing + (rand() % 4);
        data->streams[i].y = (float)(-(rand() % SCREEN_HEIGHT));
        data->streams[i].speed = 3.5f + ((rand() % 50) / 10.0f);
        data->streams[i].tail_len = 6 + (rand() % 8);
    }

    lv_obj_t * matrix_scr = lv_obj_create(NULL); 
    lv_obj_set_user_data(matrix_scr, data);

    lv_obj_set_style_bg_color(matrix_scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(matrix_scr, LV_OPA_COVER, 0);
    lv_obj_add_flag(matrix_scr, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(matrix_scr, matrix_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(matrix_scr, matrix_screen_delete_cb, LV_EVENT_DELETE, NULL);

    data->engine_timer = lv_timer_create(matrix_engine_timer_cb, 40, matrix_scr);

    lv_screen_load_anim(matrix_scr, LV_SCR_LOAD_ANIM_NONE, 100, 0, false);
}
#endif