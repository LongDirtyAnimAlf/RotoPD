#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <lvgl.h>

/* Call this once in your Arduino setup() after initializing LVGL and your inputs */
void init_screensaver_controller(uint32_t timeout_ms);

/* Call this inside your main loop to let the manager check for inactivity */
void check_screensaver_timeout(void);

/* Call this manually if any non-LVGL hardware buttons are pressed */
void reset_screensaver_idle_timer(void);

#endif // SCREENSAVER_H
