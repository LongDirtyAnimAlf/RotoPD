#ifndef MATRIX_SCREEN_H
#define MATRIX_SCREEN_H

#include "Arduino.h"
#include <lvgl.h>
#include "sevensegment.h"
#include "./../../shared.h"

#define BOX_COUNT     6

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef struct {
    float voltage; // Volts (V)
    float current; // Amperes (A)
    float power;   // Watts (W)
    float energy;  // Watt-hours (Wh)
} telemetry_data_t;
*/

/**
 * @brief Entry point called by screensaver controller on timeout
 */
void navigate_to_matrix_screen(void);

/**
 * @brief Update telemetry values from I2C reading task/loop
 */
void telemetry_screensaver_update_i2c(const PMeasurementData data);
void telemetry_screensaver_update_V(const uint16_t data);
void telemetry_screensaver_update_I(const uint16_t data);
void telemetry_screensaver_update_P(const uint32_t data);
void telemetry_screensaver_update_E(const uint32_t data);
void telemetry_screensaver_update_box5(const uint32_t data);
void telemetry_screensaver_update_box6(const uint32_t data);

void pause_matrix_screen(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // MATRIX_SCREEN_H
