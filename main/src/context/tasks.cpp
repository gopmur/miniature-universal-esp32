#include "context/control_state.hpp"
#include "tasks/dns.hpp"
#include "tasks/http.hpp"
#include "tasks/http_wifi_con_handler.hpp"
#include "tasks/imu.hpp"
#include "tasks/ws.hpp"

HttpService http_service;

ImuTask imu_task;
WebSocketTask ws_task;
WifiConHandlerTask wifi_con_handler_task;
DnsTask dns_task("192.168.4.1", CONFIG_HEXA_TASKS_DNS_NAME_SIZE);

ControlState control_state;