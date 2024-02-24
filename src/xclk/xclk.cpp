#include "xclk.h"


esp_err_t XCLK::enable_gpio(xclk_conf_t* xclk_conf) {
  gpio_config_t gpio_conf = {
    .pin_bit_mask = (1ULL << xclk_conf->pin_xclk),
    .mode         = GPIO_MODE_OUTPUT,
    .pull_up_en   = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type    = GPIO_INTR_DISABLE,
  };
  return gpio_config(&gpio_conf);
};

esp_err_t XCLK::enable_timer(xclk_conf_t* xclk_conf) {
  ledc_timer_config_t timer_conf = {
    .speed_mode      = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_1_BIT,
    .timer_num       = xclk_conf->timer,
    .freq_hz         = xclk_conf->freq_Hz,
    .clk_cfg         = LEDC_AUTO_CLK,
  };
  return ledc_timer_config(&timer_conf);
};

esp_err_t XCLK::enable_ledc(xclk_conf_t* xclk_conf) {
  ledc_channel_config_t ledc_conf = {
    .gpio_num   = xclk_conf->pin_xclk,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel    = xclk_conf->chan,
    .intr_type  = LEDC_INTR_DISABLE,
    .timer_sel  = xclk_conf->timer,
    .duty       = LEDC_TIMER_1_BIT, // 50% duty for square wave (2-bit resolution)
    .hpoint     = 0,
    .flags      = 0,
  };
  return ledc_channel_config(&ledc_conf);
};

esp_err_t XCLK::init(xclk_conf_t* xclk_conf) {
  ESP_LOGD(TAG, "Start XCLK initialization...");
  ESP_LOGD(TAG, " - Use GPIO_NUM_%d",       xclk_conf->pin_xclk);
  ESP_LOGD(TAG, " - Set frequency %ld Hz", xclk_conf->freq_Hz);
  ESP_RETURN_ON_ERROR(enable_gpio(xclk_conf),  TAG, "Failed GPIO configuration.");
  ESP_RETURN_ON_ERROR(enable_timer(xclk_conf), TAG, "Failed timer configuration.");
  ESP_RETURN_ON_ERROR(enable_ledc(xclk_conf),  TAG, "Failed LEDC configuration.");
  ESP_LOGD(TAG, "Finished XCLK initialization.");
  return ESP_OK;
};
