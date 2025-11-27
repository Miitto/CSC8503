#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace {
auto createLogger() {
  auto logger = spdlog::stdout_color_mt("CSC8503");
  logger->set_pattern("[%H:%M:%S.%e] [%n] [%^%L%$] [%s:%#] %v");
  return logger;
}
} // namespace

namespace NCL::logging {
auto mainLogger = createLogger();
} // namespace NCL::logging