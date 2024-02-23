#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include <stdint.h>
#include "driver/gpio.h"
#include "i2c/i2c.h"
#include "esp_log.h"

#define LOG_LEVEL   ESP_LOG_DEBUG
#define LOG_SCOPE   "*"

extern "C" {
  void app_main(void);
};


const i2c_conf_t i2c_conf {
  .pin_sda = GPIO_NUM_26,
  .pin_scl = GPIO_NUM_27,
};


void enable_logging() {
  esp_log_level_set(LOG_SCOPE, LOG_LEVEL);
  vTaskDelay(pdMS_TO_TICKS(500));
};

void app_main() {
  enable_logging();
  
  uint8_t val = 0x00;
  I2C::init(i2c_conf);

  while(1) {
    I2C::write_register(0x55, 0x77);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    I2C::read_register(0x3A, &val);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("Read: %d\n", val);
    fflush(stdout);
  }
};