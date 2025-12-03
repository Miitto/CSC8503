#pragma once

#include <spdlog/spdlog.h>

#ifndef PHYS_LOG_LEVEL
#define PHYS_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#ifndef NET_LOG_LEVEL
#define NET_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#ifndef AI_LOG_LEVEL
#define AI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_TRACE
#define PHYS_TRACE(...)                                                        \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::trace, __VA_ARGS__)
#else
#define PHYS_TRACE(...) (void)0
#endif
#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_DEBUG
#define PHYS_DEBUG(...)                                                        \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::debug, __VA_ARGS__)

#else
#define PHYS_DEBUG(...) (void)0
#endif
#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_INFO
#define PHYS_LOG(...)                                                          \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::info, __VA_ARGS__)
#else
#define PHYS_LOG(...) (void)0
#endif
#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_WARN
#define PHYS_WARN(...)                                                         \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::warn, __VA_ARGS__)
#else
#define PHYS_WARN(...) (void)0
#endif
#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_ERROR
#define PHYS_ERROR(...)                                                        \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::err, __VA_ARGS__)

#else
#define PHYS_ERROR(...) (void)0
#endif
#if PHYS_LOG_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define PHYS_CRITICAL(...)                                                     \
  NCL::logging::physicsLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::critical, __VA_ARGS__)
#else
#define PHYS_CRITICAL(...) (void)0
#endif

#if NET_LOG_LEVEL <= SPDLOG_LEVEL_TRACE
#define NET_TRACE(...)                                                         \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::trace, __VA_ARGS__)
#else
#define NET_TRACE(...) (void)0
#endif
#if NET_LOG_LEVEL <= SPDLOG_LEVEL_DEBUG
#define NET_DEBUG(...)                                                         \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::debug, __VA_ARGS__)

#else
#define NET_DEBUG(...) (void)0
#endif
#if NET_LOG_LEVEL <= SPDLOG_LEVEL_INFO
#define NET_INFO(...)                                                          \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::info, __VA_ARGS__)
#else
#define NET_LOG(...) (void)0
#endif
#if NET_LOG_LEVEL <= SPDLOG_LEVEL_WARN
#define NET_WARN(...)                                                          \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::warn, __VA_ARGS__)
#else
#define NET_WARN(...) (void)0
#endif
#if NET_LOG_LEVEL <= SPDLOG_LEVEL_ERROR
#define NET_ERROR(...)                                                         \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::err, __VA_ARGS__)

#else
#define NET_ERROR(...) (void)0
#endif
#if NET_LOG_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define NET_CRITICAL(...)                                                      \
  NCL::logging::networkLogger->log(                                            \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::critical, __VA_ARGS__)
#else
#define NET_CRITICAL(...) (void)0
#endif

#define NET_ASSERT(cond, ...)                                                  \
  if (!(cond)) {                                                               \
    NCL::logging::networkLogger->critical("Assertion failed: " __VA_ARGS__);   \
    abort();                                                                   \
  }

#if AI_LOG_LEVEL <= SPDLOG_LEVEL_TRACE
#define AI_TRACE(...)                                                          \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::trace, __VA_ARGS__)
#else
#define AI_TRACE(...) (void)0
#endif
#if AI_LOG_LEVEL <= SPDLOG_LEVEL_DEBUG
#define AI_DEBUG(...)                                                          \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::debug, __VA_ARGS__)

#else
#define AI_DEBUG(...) (void)0
#endif
#if AI_LOG_LEVEL <= SPDLOG_LEVEL_INFO
#define AI_INFO(...)                                                           \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::info, __VA_ARGS__)
#else
#define AI_LOG(...) (void)0
#endif
#if AI_LOG_LEVEL <= SPDLOG_LEVEL_WARN
#define AI_WARN(...)                                                           \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::warn, __VA_ARGS__)
#else
#define AI_WARN(...) (void)0
#endif
#if AI_LOG_LEVEL <= SPDLOG_LEVEL_ERROR
#define AI_ERROR(...)                                                          \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::err, __VA_ARGS__)

#else
#define AI_ERROR(...) (void)0
#endif
#if AI_LOG_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define AI_CRITICAL(...)                                                       \
  NCL::logging::aiLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::critical, __VA_ARGS__)
#else
#define AI_CRITICAL(...) (void)0
#endif

namespace NCL::logging {
extern std::shared_ptr<spdlog::logger> physicsLogger;
extern std::shared_ptr<spdlog::logger> networkLogger;
extern std::shared_ptr<spdlog::logger> aiLogger;
} // namespace NCL::logging