//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef _U_LOGGER_H_
#define _U_LOGGER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "oic_logger.h"
#include "oic_console_logger.h"

#ifdef __ANDROID__
#include <android/log.h>
#elif defined ARDUINO
#include "Arduino.h"
#include <avr/pgmspace.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

// Use the PCF macro to wrap strings stored in FLASH on the Arduino
// Example:  OC_LOG(INFO, TAG, PCF("Entering function"));
#ifdef ARDUINO
#define PCF(str)  ((PROGMEM const char *)(F(str)))
#else
#define PCF(str) str
#endif

// Max buffer size used in variable argument log function
#define MAX_LOG_V_BUFFER_SIZE (256)

// Log levels
typedef enum
{
    DEBUG = 0, INFO, WARNING, ERROR, FATAL
} LogLevel;

#ifndef ARDUINO

/**
 * Configure logger to use a context that defines a custom logger function
 *
 * @param ctx - pointer to oc_log_ctx_t struct that defines custom logging functions
 */
void OICLogConfig(oic_log_ctx_t *ctx);

/**
 * Initialize the logger.  Optional on Android and Linux.  Configures serial port on Arduino
 */
void OICLogInit();

/**
 * Called to Free dyamically allocated resources used with custom logging.
 * Not necessary if default logging is used
 *
 */
void OICLogShutdown();

/**
 * Output a log string with the specified priority level.
 * Only defined for Linux and Android
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param logStr - log string
 */
void OICLog(LogLevel level, const char * tag, const char * logStr);

/**
 * Output a variable argument list log string with the specified priority level.
 * Only defined for Linux and Android
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param format - variadic log string
 */
void OICLogv(LogLevel level, const char * tag, const char * format, ...);

/**
 * Output the contents of the specified buffer (in hex) with the specified priority level.
 *
 * @param level      - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag        - Module name
 * @param buffer     - pointer to buffer of bytes
 * @param bufferSize - max number of byte in buffer
 */
void OICLogBuffer(LogLevel level, const char * tag, const uint8_t * buffer, uint16_t bufferSize);
#else
/**
 * Initialize the serial logger for Arduino
 * Only defined for Arduino
 */
void OICLogInit();

/**
 * Output a log string with the specified priority level.
 * Only defined for Arduino.  Uses PROGMEM strings
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param logStr - log string
 */
void OICLog(LogLevel level, PROGMEM const char * tag, PROGMEM const char * logStr);

/**
 * Output the contents of the specified buffer (in hex) with the specified priority level.
 *
 * @param level      - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag        - Module name
 * @param buffer     - pointer to buffer of bytes
 * @param bufferSize - max number of byte in buffer
 */
void OICLogBuffer(LogLevel level, PROGMEM const char * tag, const uint8_t * buffer, uint16_t bufferSize);

/**
 * Output a variable argument list log string with the specified priority level.
 *
 * @param level  - DEBUG, INFO, WARNING, ERROR, FATAL
 * @param tag    - Module name
 * @param format - variadic log string
 */
void OICLogv(LogLevel level, const char * tag, const char * format, ...);
#endif

#ifdef TB_LOG
// These macros are defined for Linux, Android, and Arduino
#define OIC_LOG_INIT()    OICLogInit()
#define OIC_LOG(level, tag, logStr)  OICLog((level), (tag), (logStr))
#define OIC_LOG_BUFFER(level, tag, buffer, bufferSize)  OICLogBuffer((level), (tag), (buffer), (bufferSize))

#ifdef ARDUINO
#define OIC_LOG_CONFIG(ctx)
#define OIC_LOG_SHUTDOWN()
// Use full namespace for logInit to avoid function name collision
#define OIC_LOG_INIT()    OICLogInit()
// Don't define variable argument log function for Arduino
#define OIC_LOG_V(level, tag, ...) OICLogv((level), (tag), __VA_ARGS__)
#else
#define OIC_LOG_CONFIG(ctx)    OICLogConfig((ctx))
#define OIC_LOG_SHUTDOWN()     OICLogShutdown()
// Define variable argument log function for Linux and Android
#define OIC_LOG_V(level, tag, ...)  OICLogv((level), (tag), __VA_ARGS__)
#endif

#else
#define OIC_LOG_CONFIG(ctx)
#define OIC_LOG_SHUTDOWN()
#define OIC_LOG(level, tag, logStr)
#define OIC_LOG_V(level, tag, ...)
#define OIC_LOG_BUFFER(level, tag, buffer, bufferSize)
#define OIC_LOG_INIT()
#endif

#ifdef __cplusplus
}
#endif // __cplusplus
#endif /* _U_LOGGER_H_ */
