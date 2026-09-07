#include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

// #include "callbacks/wifi_event_handler.hpp"
#include "callbacks/wifi_event_handler.hpp"
#include "context/control_state.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.hpp"
#include "icm20948.h"
#include "icm20948_i2c.h"
#include "services/dns.hpp"
#include "services/http.hpp"
#include "services/http_wifi_con_handler.hpp"
#include "services/imu.hpp"
#include "services/ws.hpp"

ImuThread imu_thread;
HttpService http_thread;
ControlState control_state;
WebSocketService ws_service;
HttpWifiConHandlerService http_wifi_con_handler_service;
DnsService dns_thread("192.168.4.1", "hexa.lan");

class App {
  private:
  esp_err_t res;

  void setup_flash() {
    res = nvs_flash_init();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
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
                                                        wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
  }

  void setup_i2c() {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)21,
        .scl_io_num = (gpio_num_t)22,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master =
            {
                .clk_speed = 400000,
            },
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0));
  }

  void setup() {
    setup_flash();
    setup_netif();
    setup_wifi();
    // setup_i2c();

    start_tasks();
  }

  void start_tasks() {
    http_wifi_con_handler_service.start("http_con", 2, 4096);
    dns_thread.start("dns", 2, 4096);
    ws_service.start("ws", 2, 4096);
    // imu_thread.start("imu", 2, 4096);
    http_thread.start();
  }

  public:
  void run() { setup(); }
};

extern "C" void app_main() {
  App app;
  app.run();
}