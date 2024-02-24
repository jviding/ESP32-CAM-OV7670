#pragma once

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>

#define TAG           "I2C"

#define DEVICE_ADDR   0x21
#define WRITE_ADDR    ((DEVICE_ADDR << 1) | 0x00)
#define READ_ADDR     ((DEVICE_ADDR << 1) | 0x01)


struct conf_t {
  gpio_num_t pin_sda = GPIO_NUM_26; // Default to 26
  gpio_num_t pin_scl = GPIO_NUM_27; // Default to 27
  uint32_t   freq    = 200 * 1000;  // Default to 200 kHz
  i2c_port_t port    = I2C_NUM_0;   // Default to port0
};


class I2C {
  public:
    // Struct
    using i2c_conf_t = conf_t;

    // Functions
    static esp_err_t init(i2c_conf_t* i2c_conf);
    static esp_err_t write_register(uint8_t reg, uint8_t val);
    static esp_err_t read_register(uint8_t reg, uint8_t* val);

  private:
    // Variables
    static i2c_port_t port;
    static i2c_cmd_handle_t cmd;
    static SemaphoreHandle_t i2c_handle;

    // Functions
    static esp_err_t start();
    static esp_err_t stop();
    static esp_err_t write(uint8_t val);
    static esp_err_t read(uint8_t* val);
};
