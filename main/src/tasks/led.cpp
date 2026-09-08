// #include "tasks/led.hpp"
// #include "driver/gpio.h"
// #include "freertos/idf_additions.h"
// #include "hal/gpio_types.h"
// #include "service.hpp"

// LedService::LedService(int priority) : Service(priority, "led") {}

// void LedService::main() {
//   gpio_config_t led_config = {
//       .pin_bit_mask = (1ULL << GPIO_NUM_2),
//       .mode = GPIO_MODE_OUTPUT,
//       .pull_up_en = GPIO_PULLUP_DISABLE,
//       .pull_down_en = GPIO_PULLDOWN_DISABLE,
//       .intr_type = GPIO_INTR_DISABLE,
//   };

//   gpio_config(&led_config);

//   while (true) {
//     gpio_set_level(GPIO_NUM_2, 1);
//     vTaskDelay(100);
//     gpio_set_level(GPIO_NUM_2, 0);
//     vTaskDelay(100);
//   }
// }