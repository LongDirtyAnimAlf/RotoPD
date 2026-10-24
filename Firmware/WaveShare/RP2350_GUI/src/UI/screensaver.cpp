#include "screensaver.h"
#include "matrix_screen.h"
#include "Screenbase.h"

static uint32_t idle_timeout_duration = 10000; // Default 10 seconds if not overwritten
static uint32_t last_activity_time = 0;
static bool screensaver_is_active = false;

void init_screensaver_controller(uint32_t timeout_ms) {
    idle_timeout_duration = timeout_ms;
    last_activity_time = lv_tick_get();
    screensaver_is_active = false;
}

void reset_screensaver_idle_timer(void) {
    last_activity_time = lv_tick_get();
    
    // If the screensaver is running and a user touches it, wake up the UI
    if (screensaver_is_active) {
        screensaver_is_active = false;
        
        // Pause the animation timer to save MCU cycles while hidden
        pause_matrix_screen();

        // Switch back to the base screen without deleting the telemetry screen
        lv_scr_load(screenbase);
    }
}

void check_screensaver_timeout(void) {
    // 1. Ask LVGL if any input devices (touchscreen, encoders, buttons) are active
    lv_indev_t * indev = lv_indev_get_next(NULL);
    while (indev != NULL) {
        if (lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
            reset_screensaver_idle_timer();
            return; 
        }
        indev = lv_indev_get_next(indev);
    }

    // 2. If already running, no need to check timeout constraints
    if (screensaver_is_active) return;

    // 3. Evaluate elapsed time since last activity
    if (lv_tick_elaps(last_activity_time) > idle_timeout_duration) {
        screensaver_is_active = true;
        
        // Load (or resume) the persistent matrix screen
        navigate_to_matrix_screen(); 
    }
}