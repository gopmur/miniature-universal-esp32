#include "callbacks/sntp.hpp"

void SntpCallback::sync_done(struct timeval* tv) {
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  LOGI("sntp synchronization completed. current time is %s", strftime_buf);
  SystemLogger::update_log_file_name();
}