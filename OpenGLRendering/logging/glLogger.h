#pragma once

#include <spdlog/spdlog.h>

#ifndef GL_LOG_LEVEL
#define GL_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#if GL_LOG_LEVEL <= SPDLOG_LEVEL_TRACE
#define GL_TRACE(...)                                                          \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::trace, __VA_ARGS__)
#else
#define GL_TRACE(...) (void)0
#endif
#if GL_LOG_LEVEL <= SPDLOG_LEVEL_DEBUG
#define GL_DEBUG(...)                                                          \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::debug, __VA_ARGS__)

#else
#define GL_DEBUG(...) (void)0
#endif
#if GL_LOG_LEVEL <= SPDLOG_LEVEL_INFO
#define GL_LOG(...)                                                            \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::info, __VA_ARGS__)
#else
#define GL_LOG(...) (void)0
#endif
#if GL_LOG_LEVEL <= SPDLOG_LEVEL_WARN
#define GL_WARN(...)                                                           \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::warn, __VA_ARGS__)
#else
#define GL_WARN(...) (void)0
#endif
#if GL_LOG_LEVEL <= SPDLOG_LEVEL_ERROR
#define GL_ERROR(...)                                                          \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::err, __VA_ARGS__)

#else
#define GL_ERROR(...) (void)0
#endif
#if GL_LOG_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define GL_CRITICAL(...)                                                       \
  NCL::logging::glLogger->log(                                                 \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},                 \
      spdlog::level::critical, __VA_ARGS__)
#else
#define GL_CRITICAL(...) (void)0
#endif

namespace NCL::logging {
extern std::shared_ptr<spdlog::logger> glLogger;
} // namespace NCL::logging
#undef glLogger
