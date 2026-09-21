#include "callbacks/sntp.hpp"
#include <format>
#include "tasks/wifi_con_handler.hpp"

extern FILE* log_file;
extern Mutex log_file_mutex;

void SntpCallback::sync_done(struct timeval* tv) {
  time_t now;
  char strftime_buf[64];
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  LOGI("sntp synchronization completed. current time is %s", strftime_buf);
  LOGI("updating the log file's name");
  std::string log_file_name =
      std::format("/sd/{}-{}-{}.log", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  log_file_mutex.take();
  int status = fclose(log_file);
  log_file = nullptr;
  if (status != 0) {
    LOGE("closing log file failed while trying to change its name");
    goto cleanup;
  }
  LOGI("old log file closed. trying to rename it");
  status = rename("/sd/sys.log", log_file_name.c_str());
  if (status != 0) {
    LOGE("log file renaming failed trying to reopen old file");
    log_file = fopen("/sd/sys.log", "a");
    if (log_file == nullptr) {
      LOGE("log file reopening failed");
      goto cleanup;
    }
  }
  LOGI("log file renaming was successful. trying to reopen it");
  log_file = fopen(log_file_name.c_str(), "a");
  if (log_file == nullptr) {
    LOGE("could not open new log file");
    goto cleanup;
  }
  LOGI("log file reopen was successful");
cleanup:
  log_file_mutex.give();
}