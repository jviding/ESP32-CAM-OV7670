#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>

#define TAG   "DMA"


// DESCRIPTOR HEADER
// Size must be word-aligned
struct dw0_t {
  uint32_t size     : 12; // Buffer size (words in total)
  uint32_t length   : 12; // Words per transfer (1 word = 32 bits)
  uint32_t RESERVED : 6;  // Reserved bits
  uint32_t eof      : 1;  // 0: Not EOF - 1: End of linked lists
  uint32_t owner    : 1;  // 0: CPU     - 1: DMA Controller
};

// DESCRIPTOR
// Buffer address (DW1) must be word-aligned
union _dma_desc_t {
  uint32_t bytes[3];
  struct {
    dw0_t dw0;          // dw0 - Header
    uint32_t buff_addr;  // dw1 - Pointer to buffer address, must be word-aligned
    uint32_t next_desc_addr; // dw2 - Pointer to next descriptor address
  } values;
};


class DMA {
public:
  // Functions
  static esp_err_t create_inlink(size_t num_buffs, size_t buff_size, uintptr_t* inlink_addr);
  
  
  static void print();
  
private:
  // Types
  using dma_desc_t = _dma_desc_t;

  // Variables
  static dma_desc_t* descriptors;
  static SemaphoreHandle_t dma_handle;

  // Functions
  static esp_err_t take_semaphore();
  static esp_err_t free_descriptors();
  static esp_err_t create_descriptor_with_buffer(dma_desc_t* desc, size_t buff_size);
  static esp_err_t create_descriptors_with_buffers(size_t num_buffs, size_t buff_size);
  
};
