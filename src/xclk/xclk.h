#pragma once

#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>


#define TAG   "XCLK"


struct _xclk_conf_t {
  gpio_num_t     pin_xclk  = GPIO_NUM_0;
  uint32_t       freq_MHz  = 24;
  ledc_channel_t chan      = LEDC_CHANNEL_0;
  ledc_timer_t   timer     = LEDC_TIMER_0;
};


class XCLK {
  public:
    // Struct
    using xclk_conf_t = _xclk_conf_t;

    // Functions
    static esp_err_t init(xclk_conf_t* xclk_conf);

  private:
    // Functions
    static esp_err_t enable_gpio(xclk_conf_t* xclk_conf);
    static esp_err_t enable_timer(xclk_conf_t* xclk_conf);
    static esp_err_t enable_ledc(xclk_conf_t* xclk_conf);
};
