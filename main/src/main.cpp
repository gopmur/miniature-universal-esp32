#include <string.h>
#include <algorithm>

#include <freertos/FreeRTOS.h>

// #include "callbacks/wifi_event_handler.hpp"
#include "callbacks/twai.hpp"
#include "callbacks/wifi_event_handler.hpp"
#include "custom_drivers/motor.hpp"
#include "custom_drivers/motor/odrive.hpp"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "esp_twai_types.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"
#include "hal/spi_types.h"
#include "hal/uart_types.h"
#include "http_parser.h"
#include "jaythread/executable.hpp"
#include "jaythread/sync.hpp"
#include "nvs.h"
#include "nvs_flash.h"

#include "config.hpp"
#include "icm20948.h"
#include "icm20948_i2c.h"
#include "sdkconfig.h"
#include "sdmmc_cmd.h"
#include "soc/gpio_num.h"
#include "tasks/can_recv.hpp"
#include "tasks/control.hpp"
#include "tasks/dns.hpp"
#include "tasks/http.hpp"
#include "tasks/http/module.hpp"
#include "tasks/http/modules/control.hpp"
#include "tasks/http/modules/root.hpp"
#include "tasks/imu.hpp"
#include "tasks/logger.hpp"
#include "tasks/monitor.hpp"
#include "tasks/motor.hpp"
#include "tasks/wifi_con_handler.hpp"
#include "tasks/ws.hpp"

twai_node_handle_t twai;

AbstractMotorDriver* left_motor;
AbstractMotorDriver* right_motor;

CanRecvTask* can_recv_task;
ImuTask* imu_task;
WebSocketTask* ws_task;
WifiConHandlerTask* wifi_con_handler_task;
DnsTask* dns_task;
ControlTask* control_task;
MotorTask* motor_task;
LoggerTask* logger_task;
MonitorTask* monitor_task;

HttpControlModule http_control_module("control");
HttpRootModule http_root_module("api", {&http_control_module});
HttpServer http_server(&http_root_module);

class App {
  private:
  esp_err_t res;
  // const char* tag = "main";

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
    can_recv_task = new CanRecvTask();
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
    left_motor =
        new ODriveMotorDriver(CONFIG_HEXA_MOTOR_LEFT_ID, twai, 0.8, MotorDirection::BACKWARD, 0.08);
    right_motor =
        new ODriveMotorDriver(CONFIG_HEXA_MOTOR_RIGHT_ID, twai, 0.4, MotorDirection::FORWARD, 0.04);
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

  void setup_sd() {
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };
    sdmmc_card_t* card;
    const char mount_point[] = "/sd";

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.unaligned_multi_block_rw_max_chunk_size = 8;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 23,
        .miso_io_num = 19,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret =
        spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
      ESP_LOGE("main", "failed to initialize bus.");
      return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = static_cast<gpio_num_t>(5);
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);

    ESP_LOGI("main", "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
        ESP_LOGE("main",
                 "Failed to mount filesystem. "
                 "If you want the card to be formatted, set the "
                 "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
      } else {
        ESP_LOGE("main",
                 "Failed to initialize the card (%s). "
                 "Make sure SD card lines have pull-up resistors in place.",
                 esp_err_to_name(ret));
      }
      return;
    }
    ESP_LOGI("main", "Filesystem mounted");

    sdmmc_card_print_info(stdout, card);
  }

  void start_tasks() {
    motor_task = new MotorTask(left_motor, right_motor);

    wifi_con_handler_task = new WifiConHandlerTask();
    dns_task = new DnsTask("192.168.4.1", "hexa.lan");
    ws_task = new WebSocketTask();
    control_task = new ControlTask();
    imu_task = new ImuTask();
    logger_task = new LoggerTask();
    monitor_task = new MonitorTask();

    motor_task->start("motor", 2, 4096);
    can_recv_task->start("can_recv", 2, 4096);
    wifi_con_handler_task->start("http_con",
                                 CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_PRIORITY,
                                 CONFIG_HEXA_TASKS_WIFI_CON_HANDLER_STACK_SIZE);
    dns_task->start("dns", CONFIG_HEXA_TASKS_DNS_PRIORITY, CONFIG_HEXA_TASKS_DNS_STACK_SIZE);
    ws_task->start("ws", CONFIG_HEXA_TASKS_WS_PRIORITY, CONFIG_HEXA_TASKS_WS_STACK_SIZE);
    control_task->start("control",
                        CONFIG_HEXA_TASKS_CONTROL_PRIORITY,
                        CONFIG_HEXA_TASKS_CONTROL_STACK_SIZE);
    logger_task->start("logger", 2, 4096);

    imu_task->start("imu", 3, CONFIG_HEXA_TASKS_IMU_STACK_SIZE);
    monitor_task->start("monitor", 2, 4096);
  }

  void setup() {
    esp_log_level_set("*", ESP_LOG_DEBUG);
    setup_gpio();
    setup_flash();
    setup_netif();
    setup_wifi();
    setup_i2c();
    setup_twai();
    setup_motors();
    setup_sd();

    start_tasks();

    http_server.start();
    can_recv_task->bind(CONFIG_HEXA_MOTOR_LEFT_ID << 5, ~((1 << 5) - 1), left_motor);
    can_recv_task->bind(CONFIG_HEXA_MOTOR_RIGHT_ID << 5, ~((1 << 5) - 1), right_motor);
  }

  public:
  void run() { setup(); }
};

extern "C" void app_main() {
  App app;
  app.run();
}