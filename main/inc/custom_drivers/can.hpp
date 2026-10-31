#pragma once

#include "custom_drivers/can/packet.hpp"
#include "esp_err.h"
#include "esp_twai_types.h"
#include "helper/ds/period.hpp"
#include "system_logger.hpp"

class Can {
  MAKE_LOGGABLE("can_driver");

  private:
  uint8_t rx_pin;
  uint8_t tx_pin;
  Period nominal_baudrate;
  Period data_baudrate;
  uint16_t nominal_sp_permill;
  uint16_t nominal_ssp_permill;
  uint16_t data_sp_permill;
  uint16_t data_ssp_permill;
  size_t queue_len;
  bool fdcan;
  twai_node_handle_t twai;
  Mutex mutex;
  void restart();

  public:
  Can(uint8_t rx_pin,
      uint8_t tx_pin,
      Period nominal_baudrate,
      Period data_baudrate,
      uint16_t nominal_sp_permill,
      uint16_t nominal_ssp_permill,
      uint16_t data_sp_permill,
      uint16_t data_ssp_permill,
      size_t queue_len);
  Can(uint8_t rx_pin,
      uint8_t tx_pin,
      Period nominal_baudrate,
      uint16_t nominal_sp_permill,
      uint16_t nominal_ssp_permill,
      size_t queue_len);
  esp_err_t send_packet(CanPacket packet);
  void init();
};