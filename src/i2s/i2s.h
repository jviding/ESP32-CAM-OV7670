#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2s.h"
#include "soc/i2s_reg.h"
#include "esp_check.h"
#include "esp_log.h"

#define TAG       "I2S"

// 1. Sort out DMA RX EOF INT
// 2. Figure out clock? Is BCK div_num relevant?
// 3. ERR handling in FIFO ISR?
// 4. Return pointer to buffer, or copy to RAM?


struct _i2s_conf_t { 
  gpio_num_t pin_pclk  = GPIO_NUM_22;
  gpio_num_t pin_href  = GPIO_NUM_23;
  gpio_num_t pin_vsync = GPIO_NUM_25;
  gpio_num_t pin_d0    = GPIO_NUM_4;
  gpio_num_t pin_d1    = GPIO_NUM_5;
  gpio_num_t pin_d2    = GPIO_NUM_18;
  gpio_num_t pin_d3    = GPIO_NUM_19;
  gpio_num_t pin_d4    = GPIO_NUM_34;
  gpio_num_t pin_d5    = GPIO_NUM_35;
  gpio_num_t pin_d6    = GPIO_NUM_12;
  gpio_num_t pin_d7    = GPIO_NUM_13;

  uint32_t   freq     = 200 * 1000;  // Default to 200 kHz
};


class I2S {
public:
  // Struct
  using i2s_conf_t = _i2s_conf_t;

  // Functions
  static esp_err_t init(i2s_conf_t* i2s_conf);
  static void read();

private:
  // Functions - Peripheral
  static void periph_reset();
  static void periph_clear_defaults();
  static void periph_enable();

  // Functions - Interface
  static void iface_set_clock();
  static void iface_set_lcd_slave_receiving_mode();
  static void iface_set_fifo_dma_mode();
  static void iface_enable();

  // Functions - GPIO
  static esp_err_t gpio_enable_pin(gpio_num_t gpio_num);
  static esp_err_t gpio_enable(i2s_conf_t* i2s_conf);

  // Variables - Fifo
  static SemaphoreHandle_t isr_handle;
  static intr_handle_t intr_handle;
  //static uint32_t inlink_addr;
  // Functions - Fifo
  static void fifo_inlink_start();
  static void fifo_inlink_stop();
  static esp_err_t fifo_inlink_set(size_t buff_size);
  static esp_err_t fifo_inlink_enable();
  
  static void IRAM_ATTR fifo_dma_stop_ISR(void* arg);
  static void fifo_dma_read();
};
