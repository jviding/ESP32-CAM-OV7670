#include "i2s.h"
#include "../dma/dma.h"


SemaphoreHandle_t I2S::isr_handle;
intr_handle_t I2S::intr_handle;
//uint32_t I2S::inlink_addr;


void I2S::fifo_inlink_start() {
  // Reset
  SET_PERI_REG_MASK(I2S_IN_LINK_REG(0), I2S_INLINK_RESTART);
  // Then start
  CLEAR_PERI_REG_MASK(I2S_IN_LINK_REG(0), I2S_INLINK_STOP);
  SET_PERI_REG_MASK(I2S_IN_LINK_REG(0), I2S_INLINK_START);
};

void I2S::fifo_inlink_stop() {
  SET_PERI_REG_MASK(I2S_IN_LINK_REG(0), I2S_INLINK_STOP);
  CLEAR_PERI_REG_MASK(I2S_IN_LINK_REG(0), I2S_INLINK_START);
};

esp_err_t I2S::fifo_inlink_set(size_t buff_size) {
  // Allocate inlink DMA descriptors & buffers
  uintptr_t inlink_addr = 0;
  ESP_RETURN_ON_ERROR(DMA::create_inlink(1, buff_size, &inlink_addr), TAG, "Failed setting FIFO DMA inlink.");
  // Set inlink address
  SET_PERI_REG_BITS(I2S_IN_LINK_REG(0), I2S_INLINK_ADDR, inlink_addr, I2S_INLINK_ADDR_S);
  // Set expected receive size, for RX EOF INT
  uint32_t num_words = 8; //num_bytes / 2; // 16;
  WRITE_PERI_REG(I2S_RXEOF_NUM_REG(0), num_words);
  ESP_LOGD(TAG, "FIFO DMA inlink set, ready to start receiving.");
  return ESP_OK;
};

esp_err_t I2S::fifo_inlink_enable() {
  // Ensure inlink is stopped
  fifo_inlink_stop();
  // Enable RX EOF interrupt
  SET_PERI_REG_MASK(I2S_INT_ENA_REG(0), I2S_IN_SUC_EOF_INT_ENA); 
  SET_PERI_REG_MASK(I2S_INT_CLR_REG(0), I2S_IN_SUC_EOF_INT_CLR);
  // Create ISR handle
  isr_handle = xSemaphoreCreateBinary();
  ESP_RETURN_ON_FALSE(isr_handle != NULL, ESP_ERR_NO_MEM, TAG, "Failed creating FIFO DMA ISR semaphore.");
  // Done
  ESP_LOGD(TAG, "Enabled FIFO DMA inlink RX EOF INT.");
  return ESP_OK;
};

void IRAM_ATTR I2S::fifo_dma_stop_ISR(void* arg) {
  // Stop inlink
  fifo_inlink_stop();
  // Clear interrupt
  SET_PERI_REG_MASK(I2S_INT_CLR_REG(0), I2S_IN_SUC_EOF_INT_CLR);
  // Signal ISR ready
  xSemaphoreGiveFromISR(isr_handle, NULL);
};

void I2S::fifo_dma_read() {
  // Enable ISR
  esp_intr_alloc(ETS_I2S0_INTR_SOURCE, 0, fifo_dma_stop_ISR, nullptr, &intr_handle); // (intr_handler_t)I2S::
  // Start RX
  fifo_inlink_start();
  ESP_LOGD(TAG, "Started FIFO DMA inlink, receiving...");
  // Wait until RX EOF
  while (1) {
    // CPU non-blocking wait
    if (xSemaphoreTake(isr_handle, pdMS_TO_TICKS(10)) == pdTRUE) {
      // Release the ISR upon complete
      esp_intr_free(intr_handle);
      break;
    }
  };
  ESP_LOGD(TAG, "Done.");
  DMA::print();
  ESP_LOGD(TAG, "Printed.");
};
