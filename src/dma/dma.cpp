#include "dma.h"
#include "soc/i2s_reg.h"


DMA::dma_desc_t* DMA::descriptors = NULL;
SemaphoreHandle_t DMA::dma_handle = NULL;


esp_err_t DMA::take_semaphore() {
  if (dma_handle == NULL) {
    dma_handle = xSemaphoreCreateBinary();
    ESP_RETURN_ON_FALSE(dma_handle != NULL, ESP_ERR_NO_MEM, TAG, "Failed creating semaphore.");
  } else {
    ESP_RETURN_ON_FALSE(xSemaphoreTake(dma_handle, portMAX_DELAY) == pdTRUE, ESP_ERR_NO_MEM, TAG, "Failed taking semaphore.");
  }
  return ESP_OK;
};

esp_err_t DMA::free_descriptors() {
  bool eof = 0;
  size_t i = 0;
  // Iterate descriptors & free their buffers until EOF
  while (eof != 1) {
    uint32_t* buff = (uint32_t*)descriptors[i].values.buff_addr;
    heap_caps_free(buff);
    eof = (bool*)descriptors[i].values.dw0.eof;
    i++;
  }
  heap_caps_free(descriptors);
  return ESP_OK;
};

esp_err_t DMA::create_descriptor_with_buffer(dma_desc_t* desc, size_t buff_size) {
  // Allocate buffer from DMA capable memory
  uint32_t* buff = (uint32_t*)heap_caps_malloc(buff_size * sizeof(uint32_t), MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
  ESP_RETURN_ON_FALSE(buff != NULL, ESP_ERR_NO_MEM, TAG, "Failed allocating memory for DMA buffer.");
  // Populate descriptor
  desc->values.dw0 = {
    .size   = buff_size,
    .length = 1,
    .eof    = 1,
    .owner  = 1,
  };
  desc->values.buff_addr = (uintptr_t)buff;
  desc->values.next_desc_addr = 0;
  return ESP_OK;
};

esp_err_t DMA::create_descriptors_with_buffers(size_t num_buffs, size_t buff_size) {
  // Allocate DMA capable memory for descriptors
  descriptors = (dma_desc_t*)heap_caps_malloc(num_buffs * sizeof(dma_desc_t), MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
  ESP_RETURN_ON_FALSE(descriptors != NULL, ESP_ERR_NO_MEM, TAG, "Failed allocating memory for DMA descriptors.");
  // Create descriptors with buffers
  for (size_t i = 0; i < num_buffs; i++) {
    ESP_RETURN_ON_ERROR(create_descriptor_with_buffer(&descriptors[i], buff_size), TAG, "Failed creating DMA descriptor.");
    if (i > 0) {
      descriptors[i - 1].values.dw0.eof = 0;
      descriptors[i - 1].values.next_desc_addr = (uintptr_t)&descriptors[i];
    }
  }
  return ESP_OK;
};

esp_err_t DMA::create_inlink(size_t num_buffs, size_t buff_size, uintptr_t* inlink_addr) {
  ESP_LOGD(TAG, "Creating DMA descriptors for inlink...");
  ESP_RETURN_ON_ERROR(take_semaphore(), TAG, "Failed creating DMA descriptors.");
  // Free before reallocation
  if (descriptors != NULL) {
    ESP_RETURN_ON_ERROR(free_descriptors(), TAG, "Failed deallocating old DMA descriptors.");
  }
  // Create descriptors & buffers
  ESP_RETURN_ON_ERROR(create_descriptors_with_buffers(num_buffs, buff_size), TAG, "Failed creating DMA descriptors and buffers.");
  // Give inlink address
  *inlink_addr = (uintptr_t)&descriptors[0];
  // Release semaphore
  ESP_RETURN_ON_FALSE(xSemaphoreGive(dma_handle) == pdTRUE, ESP_ERR_NO_MEM, TAG, "Failed giving semaphore.");
  // Done
  ESP_LOGD(TAG, "Created %d DMA descriptors with %d-word DMA buffers.", num_buffs, buff_size);
  return ESP_OK;
};





void DMA::print() {
  bool eof = 0;
  size_t i = 0;
  while (eof != 1) {
    uint32_t* buff = (uint32_t*)descriptors[i].values.buff_addr;

    for (size_t a = 0; a < 32; a++) {
      printf("%lx\n", buff[a]);
    }

    eof = descriptors[i].values.dw0.eof;
    i++;
  }
  
  // QQVGA 160*120 (80 * 60 32-b)
  /*for (size_t a = 0; a < 80; a++) {
    for (size_t b = 0; b < 60; b++) {
      uint32_t two_pxl = buffers[0][a*80 + b];
      // Take brightness
      uint8_t frst = (two_pxl >> 24);
      uint8_t scnd = (two_pxl >> 8);
      // Print
      printf("%d %d ", frst, scnd);
    }
    printf("\n");
  }*/

  

  printf("\n\n");
  fflush(stdout);
};
