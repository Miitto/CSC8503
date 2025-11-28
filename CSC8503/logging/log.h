#pragma once

#include <spdlog/spdlog.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#if LOG_LEVEL <= SPDLOG_LEVEL_TRACE
#define TRACE(...)                                                             \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::trace, __VA_ARGS__)
#else
#define TRACE(...) (void)0
#endif
#if LOG_LEVEL <= SPDLOG_LEVEL_DEBUG
#define DEBUG(...)                                                             \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::debug, __VA_ARGS__)

#else
#define DEBUG(...) (void)0
#endif
#if LOG_LEVEL <= SPDLOG_LEVEL_INFO
#define LOG(...)                                                               \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::info, __VA_ARGS__)
#else
#define LOG(...) (void)0
#endif
#if LOG_LEVEL <= SPDLOG_LEVEL_WARN
#define WARN(...)                                                              \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::warn, __VA_ARGS__)
#else
#define WARN(...) (void)0
#endif
#if LOG_LEVEL <= SPDLOG_LEVEL_ERROR
#define ERROR(...)                                                             \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::err, __VA_ARGS__)

#else
#define ERROR(...) (void)0
#endif
#if LOG_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define CRITICAL(...)                                                          \
  NCL::logging::mainLogger->log(                                               \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::critical, __VA_ARGS__)
#else
#define CRITICAL(...) (void)0
#endif

namespace NCL::logging {
extern std::shared_ptr<spdlog::logger> mainLogger;
} // namespace NCL::logging