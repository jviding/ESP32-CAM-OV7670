#include "i2s.h"


// Freq(I2S) = F(80MHz) / (n + b/a)
void I2S::iface_set_clock() {
  // Use 48 MHz
  uint16_t a = 1; // 3
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_A, a, I2S_CLKM_DIV_A_S);
  uint16_t b = 0; // 2
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_B, b, I2S_CLKM_DIV_B_S);
  uint16_t n = 2; // 1
  SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_NUM, n, I2S_CLKM_DIV_NUM_S);
};

void I2S::iface_set_lcd_slave_receiving_mode() {
  // Enable LCD Slave receiving mode
  SET_PERI_REG_MASK(I2S_CONF_REG(0), I2S_RX_SLAVE_MOD);
  SET_PERI_REG_MASK(I2S_CONF2_REG(0), I2S_LCD_EN | I2S_CAMERA_EN);
  CLEAR_PERI_REG_MASK(I2S_CONF_REG(0), (I2S_RX_MSB_RIGHT | I2S_RX_RIGHT_FIRST));
  // Correct mode to receive data  
  SET_PERI_REG_BITS(I2S_CONF_CHAN_REG(0), I2S_RX_CHAN_MOD, 1, I2S_RX_CHAN_MOD_S);
  SET_PERI_REG_BITS(I2S_FIFO_CONF_REG(0), I2S_RX_FIFO_MOD, 1, I2S_RX_FIFO_MOD_S);
  // 8x data pins, D7:0
  SET_PERI_REG_BITS(I2S_FIFO_CONF_REG(0), I2S_RX_DATA_NUM, 8, I2S_RX_DATA_NUM_S);
};

void I2S::iface_set_fifo_dma_mode() {
  // Enable FIFO DMA mode
  SET_PERI_REG_MASK(I2S_FIFO_CONF_REG(0), I2S_RX_FIFO_MOD_FORCE_EN);
  SET_PERI_REG_MASK(I2S_FIFO_CONF_REG(0), I2S_DSCR_EN);
  // Use DMA burst mode, for fast memory access
  // Occupies system bus during RX write, blocking CPU
  SET_PERI_REG_MASK(I2S_LC_CONF_REG(0), I2S_INDSCR_BURST_EN);
  SET_PERI_REG_MASK(I2S_LC_CONF_REG(0), I2S_OUT_DATA_BURST_EN);
  // Follow descriptors (linked list) without CPU
  SET_PERI_REG_MASK(I2S_RXEOF_NUM_REG(0), I2S_OUT_AUTO_WRBACK);
};

void I2S::iface_enable() {
  iface_set_clock();
  ESP_LOGD(TAG, "Enabled I2S peripheral clock.");
  iface_set_lcd_slave_receiving_mode();
  ESP_LOGD(TAG, "Enabled I2S LCD Slave Receiving Mode.");
  iface_set_fifo_dma_mode();
  ESP_LOGD(TAG, "Enabled I2S FIFO DMA Mode.");
  // Start RX receiver
  SET_PERI_REG_MASK(I2S_CONF_REG(0), I2S_RX_START);
  ESP_LOGD(TAG, "Started RX receiver.");
};
