#include "i2s.h"


esp_err_t I2S::init(i2s_conf_t* i2s_conf) {
  ESP_LOGD(TAG, "Start I2S initialization...");
  periph_enable();
  iface_enable();
  ESP_RETURN_ON_ERROR(gpio_enable(i2s_conf),  TAG, "Failed GPIO configuration.");
  

  fifo_inlink_enable();
  
  fifo_inlink_set(100);
  
  uint32_t frame_size = 160 * 120 / 2; // QQVGA  160*120 (16-bit pixels)
  // Enable inlink and RX interrupt
  //enable_fifo_dma_inlink(frame_size);
  //enable_fifo_dma_interrupt(frame_size);
 
  //ESP_RETURN_ON_ERROR(enable_timer(xclk_conf), TAG, "Failed timer configuration.");
  //ESP_RETURN_ON_ERROR(enable_ledc(xclk_conf),  TAG, "Failed LEDC configuration.");
  
  //enable_channel();
  //enable_gpio(i2s_conf);
  //enable_fifo_dma_mode();
  
 

  ESP_LOGD(TAG, "Finished I2S initialization.");
  return ESP_OK;
};

void I2S::read() {
  ESP_LOGD(TAG, "Try read.");
  fifo_dma_read();
};