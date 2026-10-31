#include "custom_drivers/can.hpp"
#include "callbacks/twai.hpp"
#include "esp_err.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "system_logger.hpp"

Can::Can(uint8_t rx_pin,
         uint8_t tx_pin,
         Period nominal_baudrate,
         Period data_baudrate,
         uint16_t nominal_sp_permill,
         uint16_t nominal_ssp_permill,
         uint16_t data_sp_permill,
         uint16_t data_ssp_permill,
         size_t queue_len)
    : rx_pin(rx_pin),
      tx_pin(tx_pin),
      nominal_baudrate(nominal_baudrate),
      data_baudrate(data_baudrate),
      nominal_sp_permill(nominal_sp_permill),
      nominal_ssp_permill(nominal_ssp_permill),
      data_sp_permill(data_sp_permill),
      data_ssp_permill(data_ssp_permill),
      queue_len(queue_len),
      fdcan(true) {}

Can::Can(uint8_t rx_pin,
         uint8_t tx_pin,
         Period nominal_baudrate,
         uint16_t nominal_sp_permill,
         uint16_t nominal_ssp_permill,
         size_t queue_len)
    : rx_pin(rx_pin),
      tx_pin(tx_pin),
      nominal_baudrate(nominal_baudrate),
      data_baudrate(nominal_baudrate),
      nominal_sp_permill(nominal_sp_permill),
      nominal_ssp_permill(nominal_ssp_permill),
      data_sp_permill(nominal_sp_permill),
      data_ssp_permill(nominal_ssp_permill),
      queue_len(queue_len),
      fdcan(false) {}

esp_err_t Can::send_packet(CanPacket packet) {
  twai_frame_t twai_frame;
  packet.header.fdf = fdcan;
  twai_frame.header = packet.header;
  twai_frame.buffer = packet.data.data();
  twai_frame.buffer_len = packet.header.dlc;
  mutex.take();
  auto status = twai_node_transmit(twai, &twai_frame, 1);
  mutex.give();
  if (status == ESP_ERR_TIMEOUT) {
    LOGW("timeout occurred while queueing id 0x%02x", packet.header.id);
    return status;
  } else if (status != ESP_OK) {
    LOGE("unhandled error occurred while queueing id 0x%02x. %s",
         packet.header.id,
         esp_err_to_name(status));
    return status;
  }
  mutex.take();
  status = twai_node_transmit_wait_all_done(twai, 100);
  mutex.give();
  if (status == ESP_ERR_TIMEOUT) {
    LOGW("timeout occurred while transmiting id 0x%02x. check the physical connection",
         packet.header.id);
    return status;
  } else if (status == ESP_ERR_INVALID_STATE) {
    LOGE("busoff detected while transmitting id 0x%02x. reinstalling twai driver",
         packet.header.id);
    restart();
  } else if (status != ESP_OK) {
    LOGE("unhandled error occurred while transmiting id 0x%02x. %s",
         packet.header.id,
         esp_err_to_name(status));
    return status;
  }
  return status;
};

void Can::restart() {
  twai_node_disable(twai);
  twai_node_enable(twai);
}

void Can::init() {
  twai_event_callbacks_t twai_callback = {
      .on_tx_done = nullptr,
      .on_rx_done = TwaiCallback::rx_done,
      .on_state_change = nullptr,
      .on_error = nullptr,
  };
  twai_onchip_node_config_t twai_config = {
      .io_cfg =
          {
              .tx = static_cast<gpio_num_t>(tx_pin),
              .rx = static_cast<gpio_num_t>(rx_pin),
              .quanta_clk_out = static_cast<gpio_num_t>(-1),
              .bus_off_indicator = static_cast<gpio_num_t>(-1),
          },

      .clk_src = TWAI_CLK_SRC_DEFAULT,
      .bit_timing =
          {
              .bitrate = static_cast<uint32_t>(nominal_baudrate.hz()),
              .sp_permill = nominal_sp_permill,
              .ssp_permill = nominal_ssp_permill,
          },
      .data_timing =
          {
              .bitrate = static_cast<uint32_t>(data_baudrate.hz()),
              .sp_permill = data_sp_permill,
              .ssp_permill = data_ssp_permill,
          },
      .timestamp_resolution_hz = 0,
      .fail_retry_cnt = -1,
      .tx_queue_depth = queue_len,
      .intr_priority = 0,
      .flags =
          {
              .enable_self_test = 0,
              .enable_loopback = 0,
              .enable_listen_only = 0,
              .no_receive_rtr = 0,
              .sleep_allow_pd = 0,
          },
  };
  ESP_ERROR_CHECK(twai_new_node_onchip(&twai_config, &twai));
  ESP_ERROR_CHECK(twai_node_register_event_callbacks(twai, &twai_callback, nullptr));
  ESP_ERROR_CHECK(twai_node_enable(twai));
}