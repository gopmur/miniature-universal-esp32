#include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

#include "driver/uart.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.hpp"

#include "context/services/dns.hpp"
#include "context/services/http.hpp"
#include "context/services/led.hpp"
#include "context/services/monitor.hpp"
#include "context/services/stm_uart_rx.hpp"
#include "context/services/ws.hpp"

class App {
  private:
  esp_err_t res;

  void setup_flash() {
    res = nvs_flash_init();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES ||
        res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ESP_ERROR_CHECK(nvs_flash_init());
    } else {
      ESP_ERROR_CHECK(res);
    }
  }
  void setup_netif() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
  }

  static void wifi_event_handler(void* arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
      ESP_LOGI("Wifi", "Connection start");
      esp_wifi_connect();
    }

    // else if (event_base == WIFI_EVENT &&
    //          event_id == WIFI_EVENT_STA_DISCONNECTED) {
    //   ESP_LOGW("Wifi", "Disconnected");
    //   esp_wifi_connect();  // retry
    // }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
      printf("Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
    }
  }

  void setup_wifi() {
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

    wifi_config_t wifi_ap_config = {};
    std::copy(config::wifi::ssid,
              config::wifi::ssid + strlen(config::wifi::ssid),
              wifi_ap_config.ap.ssid);
    std::copy(config::wifi::password,
              config::wifi::password + strlen(config::wifi::password),
              wifi_ap_config.ap.password);
    wifi_ap_config.ap.ssid_len = strlen(config::wifi::ssid);
    wifi_ap_config.ap.channel = config::wifi::channel;
    wifi_ap_config.ap.max_connection = config::wifi::max_connection;
    wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_ap_config.ap.pmf_cfg.required = false;

    if (strlen(config::wifi::password) == 0) {
      wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
  }

  void setup_uart() {
    ESP_ERROR_CHECK(uart_set_pin(config::stm_uart::port,
                                 config::stm_uart::tx_pin,
                                 config::stm_uart::rx_pin,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
    uart_config_t uart_config = {
        .baud_rate = config::stm_uart::baud_rate,
        .data_bits = config::stm_uart::data_bits,
        .parity = config::stm_uart::parity,
        .stop_bits = config::stm_uart::stop_bits,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
    };
#pragma clang diagnostic pop
    ESP_ERROR_CHECK(uart_param_config(config::stm_uart::port, &uart_config));
    ESP_ERROR_CHECK(uart_driver_install(config::stm_uart::port,
                                        config::stm_uart::rx_buffer_size,
                                        config::stm_uart::rx_buffer_size,
                                        0,
                                        nullptr,
                                        0));
  }

  void setup() {
    setup_flash();
    setup_netif();
    setup_wifi();
    setup_uart();

    http_service.start();
    dns_service.start();
    led_service.start();
    stm_uart_rx_service.start();
    ws_service.start();
    monitor_service.start();
  }

  public:
  void run() { setup(); }
};

extern "C" void app_main() {
  App app;
  app.run();
}