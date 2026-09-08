#include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

// #include "callbacks/wifi_event_handler.hpp"
#include "callbacks/wifi_event_handler.hpp"
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
#include "sdkconfig.h"
#include "tasks/dns.hpp"
#include "tasks/http.hpp"
#include "tasks/http_wifi_con_handler.hpp"
#include "tasks/imu.hpp"
#include "tasks/ws.hpp"

extern HttpService http_service;
extern ImuTask imu_task;
extern WebSocketTask ws_task;
extern WifiConHandlerTask wifi_con_handler_task;
extern DnsTask dns_task;

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

    strcpy(reinterpret_cast<char*>(&wifi_ap_config.ap.ssid), CONFIG_HEXA_WIFI_SSID);
    strcpy(reinterpret_cast<char*>(&wifi_ap_config.ap.password), CONFIG_HEXA_WIFI_PASSWORD);

    wifi_ap_config.ap.ssid_len = strlen(CONFIG_HEXA_WIFI_SSID);
    wifi_ap_config.ap.channel = CONFIG_HEXA_WIFI_CHANNELS;
    wifi_ap_config.ap.max_connection = CONFIG_HEXA_WIFI_MAX_CONNECTIONS;
    wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_ap_config.ap.pmf_cfg.required = false;

    if (strlen(CONFIG_HEXA_WIFI_PASSWORD) == 0) {
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

    http_service.start();
  }

  void start_tasks() {
    wifi_con_handler_task.start("http_con",
                                CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_PRIORITY,
                                CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_STACK_SIZE);
    dns_task.start("dns", CONFIG_HEXA_TASKS_DNS_PRIORITY, CONFIG_HEXA_TASKS_DNS_STACK_SIZE);
    ws_task.start("ws", CONFIG_HEXA_TASKS_WS_PRIORITY, CONFIG_HEXA_TASKS_WS_STACK_SIZE);
    // imu_task.start("imu", CONFIG_HEXA_TASKS_IMU_PRIORITY, CONFIG_HEXA_TASKS_IMU_STACK_SIZE);
  }

  public:
  void run() { setup(); }
};

extern "C" void app_main() {
  App app;
  app.run();
}