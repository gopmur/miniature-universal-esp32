#include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

// #include "callbacks/wifi_event_handler.hpp"
#include "callbacks/twai.hpp"
#include "callbacks/wifi_event_handler.hpp"
#include "custom_drivers/motor/odrive.hpp"
#include "driver/gpio.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "esp_twai_types.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "hal/uart_types.h"
#include "jaythread/sync.hpp"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.hpp"
#include "icm20948.h"
#include "icm20948_i2c.h"
#include "sdkconfig.h"
#include "soc/gpio_num.h"
#include "tasks/can_recv.hpp"
#include "tasks/control.hpp"
#include "tasks/dns.hpp"
#include "tasks/http.hpp"
#include "tasks/imu.hpp"
#include "tasks/wifi_con_handler.hpp"
#include "tasks/ws.hpp"

HttpService http_service;
CanRecvTask can_recv_task;
ImuTask imu_task;
WebSocketTask ws_task;
WifiConHandlerTask wifi_con_handler_task;
DnsTask dns_task("192.168.4.1", CONFIG_HEXA_TASKS_DNS_NAME_SIZE);
ControlTask control_task;

twai_node_handle_t twai;

ODriveMotorDriver* left_motor;
ODriveMotorDriver* right_motor;

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

  void setup_twai() {
    twai_event_callbacks_t twai_callback = {
        .on_tx_done = nullptr,
        .on_rx_done = TwaiCallback::rx_done,
        .on_state_change = nullptr,
        .on_error = nullptr,
    };
    twai_onchip_node_config_t twai_config = {
        .io_cfg =
            {
                .tx = static_cast<gpio_num_t>(CONFIG_HEXA_CAN_TX_PIN),
                .rx = static_cast<gpio_num_t>(CONFIG_HEXA_CAN_RX_PIN),
                .quanta_clk_out = static_cast<gpio_num_t>(-1),
                .bus_off_indicator = static_cast<gpio_num_t>(-1),
            },

        .clk_src = TWAI_CLK_SRC_DEFAULT,
        .bit_timing =
            {
                .bitrate = CONFIG_HEXA_CAN_BAUDRATE_KHZ * 1000,
                .sp_permill = 750,
                .ssp_permill = 500,
            },
        .data_timing =
            {
                .bitrate = CONFIG_HEXA_CAN_BAUDRATE_KHZ * 1000,
                .sp_permill = 750,
                .ssp_permill = 500,
            },
        .timestamp_resolution_hz = 0,
        .fail_retry_cnt = -1,
        .tx_queue_depth = CONFIG_HEXA_CAN_TX_QUEUE_LEN,
        .intr_priority = 0,
        .flags = {
            .enable_self_test = 0,
            .enable_loopback = 0,
            .enable_listen_only = 0,
            .no_receive_rtr = 0,
            .sleep_allow_pd = 0,
        }};
    ESP_ERROR_CHECK(twai_new_node_onchip(&twai_config, &twai));
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(twai, &twai_callback, nullptr));
    ESP_ERROR_CHECK(twai_node_enable(twai));
  }

  void setup_motors() {
    left_motor = new ODriveMotorDriver(CONFIG_HEXA_MOTOR_LEFT_ID, twai);
    right_motor = new ODriveMotorDriver(CONFIG_HEXA_MOTOR_RIGHT_ID, twai);
  }

  void setup_gpio() {
    gpio_config_t power_switch = {
        .pin_bit_mask = (1ULL << GPIO_NUM_12),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&power_switch);
    gpio_set_level(GPIO_NUM_12, 1);
  }

  void init_console() {
    esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    // esp_console_cmd_t hello = {
    //     .command = "hello",
    //     .help = "Print hello",
    //     .hint = nullptr,
    //     .func = hello_cmd,
    // };

    // ESP_ERROR_CHECK(esp_console_cmd_register(&hello));

    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

    uart_config.channel = UART_NUM_0;
    uart_config.baud_rate = 115200;
    uart_config.tx_gpio_num = -1;
    uart_config.rx_gpio_num = -1;

    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.prompt = "esp> ";
    repl_config.max_cmdline_length = 256;
    repl_config.max_cmdline_args = 16;

    esp_console_repl_t* repl = nullptr;

    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
  }

  void setup() {
    setup_gpio();
    setup_flash();
    setup_netif();
    setup_wifi();
    setup_i2c();
    setup_twai();
    setup_motors();

    start_tasks();

    http_service.start();
  }

  void start_tasks() {
    can_recv_task.start("can_recv", 2, 4096);
    wifi_con_handler_task.start("http_con",
                                CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_PRIORITY,
                                CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_STACK_SIZE);
    dns_task.start("dns", CONFIG_HEXA_TASKS_DNS_PRIORITY, CONFIG_HEXA_TASKS_DNS_STACK_SIZE);
    ws_task.start("ws", CONFIG_HEXA_TASKS_WS_PRIORITY, CONFIG_HEXA_TASKS_WS_STACK_SIZE);
    control_task.start("control",
                       CONFIG_HEXA_TASKS_CONTROL_PRIORITY,
                       CONFIG_HEXA_TASKS_CONTROL_STACK_SIZE);

    imu_task.start("imu", CONFIG_HEXA_TASKS_IMU_PRIORITY, CONFIG_HEXA_TASKS_IMU_STACK_SIZE);
  }

  public:
  void run() {
    setup();

    can_recv_task.bind(CONFIG_HEXA_MOTOR_LEFT_ID << 5, ~((1 << 5) - 1), left_motor);
    can_recv_task.bind(CONFIG_HEXA_MOTOR_RIGHT_ID << 5, ~((1 << 5) - 1), right_motor);

    left_motor->send_enable_command();
    right_motor->send_enable_command();
    while (true) {
      left_motor->send_torque_command(0);
      right_motor->send_torque_command(0);
      ESP_LOGI("motor", "sent command");
      Sync::sleep(1000);
    }
  }
};

extern "C" void app_main() {
  App app;
  app.run();
}