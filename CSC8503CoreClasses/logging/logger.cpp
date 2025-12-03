#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace {
auto createLogger(const char *const name) {
  auto logger = spdlog::stdout_color_mt(name);
  logger->set_level(spdlog::level::trace);
  logger->set_pattern("[%H:%M:%S.%e] [%n] [%^%L%$] [%s:%#] %v");
  return logger;
}
} // namespace

namespace NCL::logging {
auto physicsLogger = createLogger("Physics");
auto networkLogger = createLogger("Network");
auto aiLogger = createLogger("AI");
} // namespace NCL::logging