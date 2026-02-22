#include "QnnxLog.h"

#include <chrono>

namespace qnnx {

void LogDefaultCallback(const char* fmt, QnnLog_Level_t level, uint64_t timestamp, va_list argp) {
  const char* level_str = "";
  switch (level) {
    case QNN_LOG_LEVEL_ERROR:
      level_str = "ERROR";
      break;
    case QNN_LOG_LEVEL_WARN:
      level_str = "WARNING";
      break;
    case QNN_LOG_LEVEL_INFO:
      level_str = "INFO";
      break;
    case QNN_LOG_LEVEL_DEBUG:
      level_str = "DEBUG";
      break;
    case QNN_LOG_LEVEL_VERBOSE:
      level_str = "VERBOSE";
      break;
    case QNN_LOG_LEVEL_MAX:
      level_str = "UNKNOWN";
      break;
  }

  double ms = (double)timestamp / 1000000.0;
  // To avoid interleaved messages
  {
    std::lock_guard<std::mutex> lock(sg_mutex);
    fprintf(stdout, "%8.1fms [%-7s] ", ms, level_str);
    vfprintf(stdout, fmt, argp);
    fprintf(stdout, "\n");
  }
}

// QnnxLog class implementation
QnnxLog::QnnxLog(QnnLog_Callback_t callback) : callback_(callback) {
  if (!callback) {
    callback_ = LogDefaultCallback;
  }

#ifdef QNN_ENABLE_DEBUG
  max_level_ = QNN_LOG_LEVEL_DEBUG;
#else
  max_level_ = QNN_LOG_LEVEL_INFO;
#endif

  std::lock_guard<std::mutex> lock(log_mutex_);
  if ((max_level_ > QNN_LOG_LEVEL_VERBOSE) || (max_level_ == 0)) {
    if (status_) {
      status_ = QNN_LOG_ERROR_INVALID_ARGUMENT;
    }
    throw std::runtime_error("Invalid log level");
  }
  status_ = QNN_LOG_NO_ERROR;
  QNNX_INFO("Logger initialized");
}

QnnxLog::~QnnxLog() {}

uint64_t QnnxLog::GetTimeStamp() const {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace qnnx
