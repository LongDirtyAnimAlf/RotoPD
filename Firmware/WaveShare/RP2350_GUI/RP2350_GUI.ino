#include <Arduino.h>
#include "pico/stdlib.h"
#include "./src/bsp/bsp_buzzer.h"
#include "./src/bsp/bsp_battery.h"

void setup() {
  //bsp_buzzer_init();
  bsp_battery_init();
}

void loop() {
  Serial.begin();

  //bsp_buzzer_enable(true);
  delay(500);
  //bsp_buzzer_enable(false);
  delay(500);

  float voltage;
  uint16_t adc_raw;

  bsp_battery_read(&voltage, &adc_raw);  
  Serial.printf("%f.\r\n",voltage);

}
