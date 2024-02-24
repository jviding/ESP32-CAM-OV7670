#include "i2c.h"


i2c_port_t I2C::port;
i2c_cmd_handle_t I2C::cmd;
SemaphoreHandle_t I2C::i2c_handle;


esp_err_t I2C::init(i2c_conf_t* i2c_conf) {
  ESP_LOGD(TAG, "Start I2C initialization...");
  ESP_LOGD(TAG, " - sda: gpio %d", i2c_conf->pin_sda);
  ESP_LOGD(TAG, " - scl: gpio %d", i2c_conf->pin_scl);
  ESP_LOGD(TAG, " - freq: %ld hz",    i2c_conf->freq);
  ESP_LOGD(TAG, " - port: %d",     i2c_conf->port);
  // Config
  const i2c_config_t conf = {
    .mode =           I2C_MODE_MASTER,
    .sda_io_num =     i2c_conf->pin_sda,
    .scl_io_num =     i2c_conf->pin_scl,
    .sda_pullup_en =  GPIO_PULLUP_ENABLE,
    .scl_pullup_en =  GPIO_PULLUP_ENABLE,
    .master = { 
      .clk_speed =    i2c_conf->freq,
    },
    .clk_flags =      0,
  };
  // Configure and enable
  ESP_RETURN_ON_ERROR(i2c_param_config(i2c_conf->port, &conf), TAG, "Failed configuring I2C bus.");
  ESP_RETURN_ON_ERROR(i2c_driver_install(i2c_conf->port, I2C_MODE_MASTER, 0, 0, 0), TAG, "Failed installing I2C driver.");
  // Create and enable Semaphore
  i2c_handle = xSemaphoreCreateBinary();
  ESP_RETURN_ON_FALSE(i2c_handle != NULL, ESP_ERR_NO_MEM, TAG, "Failed creating I2C semaphore.");
  ESP_RETURN_ON_FALSE(xSemaphoreGive(i2c_handle) == pdTRUE, ESP_ERR_NO_MEM, TAG, "Failed giving I2C semaphore upon initialization.");
  // Set port
  port = i2c_conf->port;
  // Done
  ESP_LOGD(TAG, "Finished I2C initialization.");
  return ESP_OK;
};

esp_err_t I2C::start() {
  cmd = i2c_cmd_link_create();
  ESP_RETURN_ON_FALSE(cmd != NULL, ESP_ERR_NO_MEM, TAG, "Failed creating I2C command list.");
  ESP_RETURN_ON_ERROR(i2c_master_start(cmd), TAG, "Failed pushing 'Start signal' to I2C command list.");
  return ESP_OK;
};

esp_err_t I2C::stop() {
  ESP_RETURN_ON_ERROR(i2c_master_stop(cmd), TAG, "Failed pushing 'Stop signal' to I2C command list.");
  ESP_RETURN_ON_ERROR(i2c_master_cmd_begin(port, cmd, portMAX_DELAY), TAG, "Failed transmitting I2C command list.");
  i2c_cmd_link_delete(cmd);
  return ESP_OK;
};

esp_err_t I2C::write(uint8_t val) {
  ESP_RETURN_ON_ERROR(i2c_master_write_byte(cmd, val, false), TAG, "Failed pushing 'write byte(%02x)' to I2C command list", val);
  return ESP_OK;
};

esp_err_t I2C::read(uint8_t* val) {
  ESP_RETURN_ON_ERROR(i2c_master_read_byte(cmd, val, I2C_MASTER_NACK), TAG, "Failed pushing 'read byte' to I2C command list.");
  return ESP_OK;
};

esp_err_t I2C::write_register(uint8_t reg, uint8_t val) {
  ESP_LOGD(TAG, "Called write_register(reg: %02x, val: %02x)...", reg, val);
  ESP_RETURN_ON_FALSE(xSemaphoreTake(i2c_handle, portMAX_DELAY) == pdTRUE, ESP_ERR_TIMEOUT, TAG, "Failed taking I2C semaphore for writing.");
  ESP_RETURN_ON_ERROR(start(),           TAG, "Failed I2C start in write_register.");
  ESP_RETURN_ON_ERROR(write(WRITE_ADDR), TAG, "Failed I2C write (write address) in write_register.");
  ESP_RETURN_ON_ERROR(write(reg),        TAG, "Failed I2C write (register address) in write_register.");
  ESP_RETURN_ON_ERROR(write(val),        TAG, "Failed I2C write (register value) in write_register.");
  ESP_RETURN_ON_ERROR(stop(),            TAG, "Failed I2C stop in write_register.");
  ESP_RETURN_ON_FALSE(xSemaphoreGive(i2c_handle) == pdTRUE, ESP_ERR_NO_MEM, TAG, "Failed giving I2C semaphore after writing.");
  // Done
  ESP_LOGD(TAG, "Successfully wrote the value to register.");
  return ESP_OK;
};

esp_err_t I2C::read_register(uint8_t reg, uint8_t* val) {
  ESP_LOGD(TAG, "Called read_register(reg: %02X, *val)...", reg);
  ESP_RETURN_ON_FALSE(xSemaphoreTake(i2c_handle, portMAX_DELAY) == pdTRUE, ESP_ERR_TIMEOUT, TAG, "Failed taking I2C semaphore for reading.");
  ESP_RETURN_ON_ERROR(start(),           TAG, "Failed I2C first start in read_register.");
  ESP_RETURN_ON_ERROR(write(WRITE_ADDR), TAG, "Failed I2C write (write address) in read_register.");
  ESP_RETURN_ON_ERROR(write(reg),        TAG, "Failed I2C write (register address) in read_register.");
  ESP_RETURN_ON_ERROR(stop(),            TAG, "Failed I2C first stop in read_register.");
  ESP_RETURN_ON_ERROR(start(),           TAG, "Failed I2C second start in read_register.");
  ESP_RETURN_ON_ERROR(write(READ_ADDR),  TAG, "Failed I2C write (read address) in read_register.");
  ESP_RETURN_ON_ERROR(read(val),         TAG, "Failed I2C read (register value) in read_register.");
  ESP_RETURN_ON_ERROR(stop(),            TAG, "Failed I2C second stop in read_register.");
  ESP_RETURN_ON_FALSE(xSemaphoreGive(i2c_handle) == pdTRUE, ESP_ERR_NO_MEM, TAG, "Failed giving I2C semaphore after reading.");
  // Done
  ESP_LOGD(TAG, "Successfully read the value from register.");
  return ESP_OK;
};
