#pragma once

struct ReportRecord {
  float callback_time;
  float smc_mr_pos;
  float smc_ml_pos;
  float smc_mr_vel;
  float smc_ml_vel;

  ReportRecord();
};