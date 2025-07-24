  #include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "http.hpp"

#include "config.hpp"

class WebServer {
 private:
  httpd_handle_t server_instance;

  static esp_err_t index_html_get(httpd_req_t *req) {
    extern const char index_html_start[] asm("_binary_"
                                             "index_html"
                                             "_start");
    extern const char index_html_file_end[] asm("_binary_"
                                                "index_html"
                                                "_end");
    const size_t size = index_html_file_end - index_html_start;
    esp_err_t res = httpd_resp_set_type(req, "text/html");
    if (res) {
      return res;
    }
    return httpd_resp_send(req, index_html_start, size);
  };
  DEFINE_FILE_GET_HANDLER(manifest_webmanifest, "application/manifest+json");
  DEFINE_FILE_GET_HANDLER(registerSW_js, "application/javascript");
  DEFINE_FILE_GET_HANDLER(sw_js, "application/javascript");
  DEFINE_FILE_GET_HANDLER(workbox_5ffe50d4_js, "application/javascript");
  DEFINE_FILE_GET_HANDLER(icon_192_png, "image/png");
  DEFINE_FILE_GET_HANDLER(icon_512_png, "image/png");

 public:
  WebServer() {
    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(httpd_start(&server_instance, &http_config));
    REGISTER_FILE_URI(server_instance, index_html, "/");
    REGISTER_FILE_URI(server_instance, manifest_webmanifest,
                      "/manifest.webmanifest");
    REGISTER_FILE_URI(server_instance, registerSW_js, "/registerSW.js");
    REGISTER_FILE_URI(server_instance, sw_js, "/sw.js");
    REGISTER_FILE_URI(server_instance, workbox_5ffe50d4_js,
                      "/workbox-5ffe50d4.js");
    REGISTER_FILE_URI(server_instance, icon_192_png, "/icons/icon-192.png");
    REGISTER_FILE_URI(server_instance, icon_512_png, "/icons/icon-512.png");
  }
};

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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
  }

  void setup() {
    setup_flash();
    setup_netif();
    setup_wifi();
    WebServer webserver;
  }

 public:
  void run() { setup(); }
};

extern "C" void app_main() {
  App app;
  app.run();
}