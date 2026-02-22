#pragma once

#include <mutex>
#include <string>

#include "Log.h"
#include "QnnLog.h"

namespace qnnx {

static std::mutex sg_mutex;

// In non-hexagon app stdout is used and for hexagon farf logging is used
void LogDefaultCallback(const char* fmt, QnnLog_Level_t level, uint64_t timestamp, va_list argp);

class QnnxLog {
 public:
  QnnxLog(QnnLog_Callback_t callback);
  ~QnnxLog();

  QnnxLog(const QnnxLog&) = delete;
  QnnxLog& operator=(const QnnxLog&) = delete;
  QnnxLog(QnnxLog&&) = delete;
  QnnxLog& operator=(QnnxLog&&) = delete;

  void SetMaxLevel(QnnLog_Level_t max_level) {
    max_level_.store(max_level, std::memory_order_seq_cst);
  }

  QnnLog_Level_t GetMaxLevel() { return max_level_.load(std::memory_order_seq_cst); }

  QnnLog_Callback_t GetLogCallback() { return callback_; }

 private:
  QnnLog_Callback_t callback_;
  std::atomic<QnnLog_Level_t> max_level_;
  QnnLog_Error_t status_;
  //   uint64_t epoch_;
  std::mutex log_mutex_;
};

}  // namespace qnnx
