/**
 * @file logger.hpp
 * @author Mochalov Anton (xoore4ik@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 * Модуль для поддержки современного способа логгирования  в проекте
 * Использование модуля очень легко, Вам необходимы лишь эти макросы
 */

#ifndef H_H1_LOGGER_c97039f889419433ec1c11a4e8f5a548
#define H_H1_LOGGER_c97039f889419433ec1c11a4e8f5a548

#include "./code_location.hpp"
#include "./config.hpp"
#include "./di.hpp"
#include "h1/common.hpp"

// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_INTERNAL_INIT_CONFIG h1::log::getLoggerConfig()

// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_INTERNAL_CODE_LOCATION                                          \
    h1::log::CodeLocation                                                      \
    {                                                                          \
        static_cast<int>(__LINE__), static_cast<const char*>(__FILE__),        \
            static_cast<const char*>(__FUNCTION__)                             \
    }
// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_INTERNAL_PROJECT_LOG(level, msg)                                \
    do                                                                         \
    {                                                                          \
        h1::log::getProjectLogger(H1_LOG_INTERNAL_INIT_CONFIG)                 \
            ->level(msg, H1_LOG_INTERNAL_CODE_LOCATION);                       \
    } while (0);
// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_INFO(msg) H1_LOG_INTERNAL_PROJECT_LOG(info, msg)
// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_DEBUG(msg) H1_LOG_INTERNAL_PROJECT_LOG(debug, msg)
// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_WARN(msg) H1_LOG_INTERNAL_PROJECT_LOG(warn, msg)
// NOLINTNEXTLINE (cppcoreguidelines-macro-usage)
#define H1_LOG_ERROR(msg) H1_LOG_INTERNAL_PROJECT_LOG(error, msg)

#endif