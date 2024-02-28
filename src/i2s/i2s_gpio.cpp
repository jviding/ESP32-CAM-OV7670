#include "i2s.h"
#include "hal/gpio_hal.h"
#include "rom/gpio.h"


struct pin_conf_t {
  const char* name;
  gpio_num_t  num;
  uint32_t    signal_idx;
  bool        inverted;
};

esp_err_t I2S::gpio_enable_pin(gpio_num_t gpio_num) {
  gpio_config_t gpio_conf = {
    .pin_bit_mask = (1ULL << gpio_num),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  return gpio_config(&gpio_conf);
};

esp_err_t I2S::gpio_enable(i2s_conf_t* i2s_conf) {
  /*
  * In Camera Slave Receiving mode, master transmits when:
  * transmission_start = (HSYNC = 1) & (VSYNC = 1) & (HREF = 1)
  * 
  * With OV7670, this requires:
  * - VSYNC: Inverted
  * - HSYNC: Always high (missing from camera pin out)
  */
  pin_conf_t pin_conf[] = {
    { "PCLK",   i2s_conf->pin_pclk,  I2S0I_WS_IN_IDX,    0 },
    { "HREF",   i2s_conf->pin_href,  I2S0I_H_ENABLE_IDX, 0 },
    // Invert VSYNC
    { "VSYNC",  i2s_conf->pin_vsync, I2S0I_V_SYNC_IDX,   1 },
    // For D7:0, use matrix signals [N-1:0], where N is bit width
    { "D0",     i2s_conf->pin_d0,    I2S0I_DATA_IN0_IDX, 0 },
    { "D1",     i2s_conf->pin_d1,    I2S0I_DATA_IN1_IDX, 0 },
    { "D2",     i2s_conf->pin_d2,    I2S0I_DATA_IN2_IDX, 0 },
    { "D3",     i2s_conf->pin_d3,    I2S0I_DATA_IN3_IDX, 0 },
    { "D4",     i2s_conf->pin_d4,    I2S0I_DATA_IN4_IDX, 0 },
    { "D5",     i2s_conf->pin_d5,    I2S0I_DATA_IN5_IDX, 0 },
    { "D6",     i2s_conf->pin_d6,    I2S0I_DATA_IN6_IDX, 0 },
    { "D7",     i2s_conf->pin_d7,    I2S0I_DATA_IN7_IDX, 0 },
  };
  ESP_LOGD(TAG, "Enabling GPIO...");
  for (size_t i = 0; i < sizeof(pin_conf) / sizeof(pin_conf_t); i++) {
    ESP_LOGD(TAG, " - Set %s (gpio_num_%d) as input.", pin_conf[i].name, pin_conf[i].num);
    ESP_RETURN_ON_ERROR(gpio_enable_pin(pin_conf[i].num), TAG, "Failed configuring %s.", pin_conf[i].name);
    gpio_matrix_in(pin_conf[i].num, pin_conf[i].signal_idx, pin_conf[i].inverted);
  }
  // Set HSYNC always high
  ESP_LOGD(TAG, " - Set HSYNC signal always high.");
  gpio_matrix_in(0x38, I2S0I_H_SYNC_IDX, 0); // 0x30 sends 0, 0x38 sends 1
  // Done
  ESP_LOGD(TAG, "Enabled GPIO.");
  return ESP_OK;
};
