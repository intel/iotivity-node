/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../common.h"

extern "C" {
#include <ocstack.h>
}

#include <limits.h>

static napi_ref context = nullptr;
static napi_ref jsOpen = nullptr;
static napi_ref jsClose = nullptr;
static napi_ref jsRead = nullptr;
static napi_ref jsWrite = nullptr;
static napi_ref jsUnlink = nullptr;
static napi_env contextEnv = nullptr;

static void noopDeleter(napi_env env, void *, void *) {}

#define STORAGE_PREAMBLE(method, message, failReturn, argc)                  \
  JS_ASSERT((method), std::string("Callback for ") + message + " not found", \
            THROW_BODY(scope.env, (failReturn)));                            \
  napi_value jsContext, jsCallback, jsReturnValue;                           \
  NAPI_CALL(contextEnv,                                                      \
            napi_get_reference_value(scope.env, context, &jsContext),        \
            THROW_BODY(scope.env, (failReturn)));                            \
  NAPI_CALL(contextEnv,                                                      \
            napi_get_reference_value(scope.env, (method), &jsCallback),      \
            THROW_BODY(scope.env, (failReturn)));                            \
  napi_value arguments[argc]

static FILE *defaultOpen(const char *path, const char *mode) {
  FILE *failReturn = nullptr;
  DECLARE_HANDLE_SCOPE(scope, contextEnv, failReturn);

  STORAGE_PREAMBLE(jsOpen, "open()", failReturn, 2);

  NAPI_CALL(contextEnv, napi_create_string_utf8(scope.env, path, strlen(path),
                                                &arguments[0]),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(contextEnv, napi_create_string_utf8(scope.env, mode, strlen(mode),
                                                &arguments[1]),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(contextEnv, napi_call_function(scope.env, jsContext, jsCallback, 2,
                                           arguments, &jsReturnValue),
            THROW_BODY(scope.env, failReturn));

  J2C_DECLARE_VALUE_JS(int32_t, cResult, scope.env, jsReturnValue, napi_number,
                       "open() return value", int32, int32_t,
                       THROW_BODY(scope.env, failReturn));

  return ((FILE *)(cResult < 0 ? nullptr : new int32_t(cResult)));
}

static size_t readWrite(void *ptr, size_t size, size_t count, FILE *stream,
                        const char *operation, napi_ref method) {
  size_t failReturn = 0;
  DECLARE_HANDLE_SCOPE(scope, contextEnv, failReturn);

  size_t totalSize = size * count;
  JS_ASSERT(totalSize <= UINT32_MAX,
            std::string("Cannot ") + operation + " more than UINT32_MAX bytes",
            THROW_BODY(scope.env, failReturn));

  STORAGE_PREAMBLE(method, std::string(operation) + "()", failReturn, 3);

  NAPI_CALL(contextEnv,
            napi_create_external_buffer(scope.env, totalSize, (char *)ptr,
                                        noopDeleter, nullptr, &arguments[0]),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(contextEnv,
            napi_create_number(scope.env, (double)totalSize, &arguments[1]),
            THROW_BODY(scope.env, failReturn));
  NAPI_CALL(contextEnv,
            napi_create_number(scope.env, ((double)(*((int32_t *)stream))),
                               &arguments[2]),
            THROW_BODY(scope.env, failReturn));

  NAPI_CALL(contextEnv, napi_call_function(scope.env, jsContext, jsCallback, 3,
                                           arguments, &jsReturnValue),
            THROW_BODY(scope.env, failReturn));

  J2C_DECLARE_VALUE_JS(size_t, cResult, scope.env, jsReturnValue, napi_number,
                       std::string(operation) + "() return value", uint32,
                       uint32_t, THROW_BODY(scope.env, failReturn));

  return cResult;
}

static size_t defaultRead(void *ptr, size_t size, size_t count, FILE *stream) {
  return readWrite(ptr, size, count, stream, "read", jsRead);
}

static size_t defaultWrite(const void *ptr, size_t size, size_t count,
                           FILE *stream) {
  return readWrite((void *)ptr, size, count, stream, "write", jsWrite);
}

#define CLOSE_UNLINK(method, message, suffix, ...)                           \
  int failReturn = -1;                                                       \
  DECLARE_HANDLE_SCOPE(scope, contextEnv, failReturn);                       \
  STORAGE_PREAMBLE((method), message "()", failReturn, 1);                   \
  NAPI_CALL(contextEnv,                                                      \
            napi_create_##suffix(scope.env, __VA_ARGS__, &arguments[0]),     \
            THROW_BODY(scope.env, failReturn));                              \
  NAPI_CALL(contextEnv, napi_call_function(scope.env, jsContext, jsCallback, \
                                           1, arguments, &jsReturnValue),    \
            THROW_BODY(scope.env, failReturn));                              \
  J2C_DECLARE_VALUE_JS(int, cResult, scope.env, jsReturnValue, napi_number,  \
                       "close() return value", int32, int32_t,               \
                       THROW_BODY(scope.env, failReturn));

static int defaultClose(FILE *stream) {
  CLOSE_UNLINK(jsClose, "close", number, (double)*((int32_t *)stream));
  if (cResult != -1) {
    delete ((double *)stream);
  }
  return cResult;
}

static int defaultUnlink(const char *path) {
  CLOSE_UNLINK(jsUnlink, "unlink", string_utf8, path, strlen(path));
  return cResult;
}

#define UPDATE_REFERENCE(env, reference, local)                    \
  if ((reference)) {                                               \
    NAPI_CALL_THROW(env, napi_delete_reference(env, (reference))); \
  }                                                                \
  NAPI_CALL_THROW(env, napi_create_reference(env, (local), 1, &(reference)));

#define GET_AND_VALIDATE(varNameSuffix, env, source, name)                    \
  J2C_DECLARE_PROPERTY_JS_THROW(local##varNameSuffix, (env), (source), name); \
  J2C_VALIDATE_VALUE_TYPE_THROW((env), local##varNameSuffix, napi_function,   \
                                "handler." name);                             \
  UPDATE_REFERENCE(env, js##varNameSuffix, local##varNameSuffix)

static OCPersistentStorage storage = {defaultOpen, defaultRead, defaultWrite,
                                      defaultClose, defaultUnlink};

napi_value bind_OCRegisterPersistentStorageHandler(napi_env env,
                                                   napi_callback_info info) {
  bool callNative = !context;

  J2C_DECLARE_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_object, "handler");
  GET_AND_VALIDATE(Open, env, arguments[0], "open");
  GET_AND_VALIDATE(Read, env, arguments[0], "read");
  GET_AND_VALIDATE(Write, env, arguments[0], "write");
  GET_AND_VALIDATE(Close, env, arguments[0], "close");
  GET_AND_VALIDATE(Unlink, env, arguments[0], "unlink");
  UPDATE_REFERENCE(env, context, arguments[0]);
  contextEnv = env;

  OCStackResult result = OC_STACK_OK;
  if (callNative) {
    result = OCRegisterPersistentStorageHandler(&storage);
  }
  C2J_SET_RETURN_VALUE(env, info, number, ((double)result));
}
