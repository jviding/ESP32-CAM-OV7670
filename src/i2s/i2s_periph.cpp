#include "i2s.h"
#include "driver/periph_ctrl.h"


void I2S::periph_reset() {
  // Reset LC_CONF
  uint32_t lc_conf_reset_bits = I2S_AHBM_RST | I2S_AHBM_FIFO_RST | I2S_OUT_RST | I2S_IN_RST;
  SET_PERI_REG_MASK(I2S_LC_CONF_REG(0), lc_conf_reset_bits);
  CLEAR_PERI_REG_MASK(I2S_LC_CONF_REG(0), lc_conf_reset_bits);
  // Reset CONF
  uint32_t conf_reset_bits = I2S_RX_RESET | I2S_TX_RESET | I2S_RX_FIFO_RESET | I2S_TX_FIFO_RESET;
  SET_PERI_REG_MASK(I2S_CONF_REG(0), conf_reset_bits);
  CLEAR_PERI_REG_MASK(I2S_CONF_REG(0), conf_reset_bits);
  // Wait for reset complete
  uint32_t reset_status_bits = I2S_RX_FIFO_RESET_BACK_S | I2S_TX_FIFO_RESET_BACK_S;
  while ((READ_PERI_REG(I2S_FIFO_CONF_REG(0)) & reset_status_bits) != 0) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
};

void I2S::periph_clear_defaults() {
  CLEAR_PERI_REG_MASK(I2S_CONF_REG(0), I2S_RX_MONO | I2S_TX_MONO | I2S_RX_SHORT_SYNC | I2S_TX_SHORT_SYNC);
  CLEAR_PERI_REG_MASK(I2S_PDM_CONF_REG(0), I2S_PDM2PCM_CONV_EN | I2S_PCM2PDM_CONV_EN);
};

void I2S::periph_enable() {
  periph_module_enable(PERIPH_I2S0_MODULE);
  ESP_LOGD(TAG, "I2S peripheral started.");
  // Reset and clear
  ESP_LOGD(TAG, "Resetting I2S peripheral...");
  periph_reset();
  ESP_LOGD(TAG, "I2S peripheral reset complete.");
  periph_clear_defaults();
  ESP_LOGD(TAG, "Cleared some default bits from I2S registers.");
};
