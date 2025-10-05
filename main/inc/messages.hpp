#pragma once

#include "report.hpp"
template <typename T, typename U>
struct QueueMessage {
  T type;
  U value;
};


enum HttpToStmUartSignal {
  REQUEST_REPORT,
};

enum StmUartToHttpMessageType {
  SENDING,
  END,
};

typedef QueueMessage<StmUartToHttpMessageType, ReportRecord>
    StmUartToHttpMessage;
