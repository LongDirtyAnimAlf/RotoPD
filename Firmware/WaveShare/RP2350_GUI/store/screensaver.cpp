#include "screensaver.h"
#include "matrix_screen.h"

#include "ui.h"

static uint32_t idle_timeout_duration = 10000; // Default 10 seconds if not overwritten
static uint32_t last_activity_time = 0;
static bool screensaver_is_active = false;

/* Simple placeholder function to recreate your home interface (define this in your main UI controller) */
//extern lv_obj_t * create_home_screen(void); 

void reset_screensaver_idle_timer(void) {
    last_activity_time = lv_tick_get();
    
    // If the screensaver is running and a user touches it, wake up the UI
    if(screensaver_is_active) {
        screensaver_is_active = false;
        
        // Build your normal homepage screen
        //lv_obj_t * home_scr = create_home_screen();
        // Load the main screen and cleanly delete the Matrix screensaver screen (true)
        //lv_screen_load_anim(home_scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);

        lv_screen_load_anim(screenbase, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);
    }
}

void init_screensaver_controller(uint32_t timeout_ms) {
    idle_timeout_duration = timeout_ms;
    last_activity_time = lv_tick_get();
    screensaver_is_active = false;
}

void check_screensaver_timeout(void) {
    // 1. Ask LVGL if any input devices (touchscreen, encoders, buttons) are being interacted with
    lv_indev_t * indev = lv_indev_get_next(NULL);
    while(indev != NULL) {
        // If the state of the touch screen is pressed or changing, user is active
        if(lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED) {
            reset_screensaver_idle_timer();
            return; 
        }
        indev = lv_indev_get_next(indev);
    }

    // 2. If already running, we don't need to check timeout constraints anymore
    if(screensaver_is_active) return;

    // 3. Evaluate elapsed time since the last recorded activity flag
    if(lv_tick_elaps(last_activity_time) > idle_timeout_duration) {
        screensaver_is_active = true;
        
        // Fire the matrix canvas script we built previously
        navigate_to_matrix_screen(); 
    }
}
