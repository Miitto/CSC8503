#include "logging/glLogger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace {
auto createLogger() {
  auto logger = spdlog::stdout_color_mt("GL");
  logger->set_level(spdlog::level::trace);
  logger->set_pattern("[%H:%M:%S.%e] [%n] [%^%L%$] [%s:%#] %v");
  return logger;
}
} // namespace

namespace NCL::logging {
auto glLogger = createLogger();
} // namespace NCL::logging
