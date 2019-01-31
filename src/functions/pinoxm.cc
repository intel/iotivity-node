#include <string.h>
extern "C" {
#include <experimental/ocrandom.h>
}
#include "../src/structures/handles.h"
#include "common.h"

static JSCallback* SetInputPinWithContextCB_data = nullptr;
static JSCallback* SetDisplayPinWithContextCB_data = nullptr;
static JSCallback* SetClosePinDisplayCB_data = nullptr;

static void default_SetClosePinDisplayCB() {
  JSCallback* cData = SetClosePinDisplayCB_data;
  NapiHandleScope scope(cData->env);
  napi_value js_cb;
  NAPI_CALL(cData->env,
            napi_get_reference_value(cData->env, cData->callback, &js_cb),
            THROW_BODY(cData->env, ));
  NAPI_CALL(cData->env,
            napi_call_function(cData->env, js_cb, js_cb, 0, nullptr, nullptr));
}

static void default_SetInputPinWithContextCB(OicUuid_t deviceId,
                                             char* pinBuffer,
                                             size_t pinBufferSize,
                                             void* context) {
  JSCallback* cData = static_cast<JSCallback*>(context);
  NapiHandleScope scope(cData->env);
  HELPER_CALL(scope.open(), THROW_BODY(scope.env, ));
  char uuid_string[UUID_STRING_SIZE];

  if (!OCConvertUuidToString(deviceId.id, uuid_string)) {
    NAPI_CALL(cData->env, napi_throw_error(cData->env, "iotivity-node",
                                           "Failed to convert UUID to string"),
              THROW_BODY(cData->env, ));
    return;
  }
  napi_value argv[2];
  NAPI_CALL(cData->env, napi_create_string_utf8(cData->env, uuid_string,
                                                UUID_STRING_SIZE, &argv[0]),
            THROW_BODY(cData->env, ));
  NAPI_CALL(cData->env, napi_create_uint32(cData->env, pinBufferSize, &argv[1]),
            THROW_BODY(cData->env, ));
  napi_value js_cb;
  NAPI_CALL(cData->env,
            napi_get_reference_value(cData->env, cData->callback, &js_cb),
            THROW_BODY(cData->env, ));
  napi_value result;
  NAPI_CALL(cData->env,
            napi_call_function(cData->env, js_cb, js_cb, 2, argv, &result),
            THROW_BODY(cData->env, ));
  J2C_GET_STRING_TRACKED_JS(js_pin, cData->env, result, false, "PIN",
                            THROW_BODY(cData->env, ));
  size_t js_pin_length = strlen(js_pin);
  if (js_pin_length > pinBufferSize) {
    NAPI_CALL(cData->env,
              napi_throw_error(cData->env, "iotivity-node", "PIN is too long"),
              THROW_BODY(cData->env, ));
    return;
  }
  memcpy(pinBuffer, js_pin, js_pin_length);
  // TODO: Validate that result is a string and that it's the right length and
  // then fill out pinBuffer with its contents.
}

static void default_SetDisplayPinWithContextCB(char* pinData, size_t pinSize,
                                               void* context) {
  JSCallback* cData = static_cast<JSCallback*>(context);
  NapiHandleScope scope(cData->env);
  HELPER_CALL(scope.open(), THROW_BODY(scope.env, ));
  napi_value js_cb;
  NAPI_CALL(cData->env,
            napi_get_reference_value(cData->env, cData->callback, &js_cb),
            THROW_BODY(cData->env, ));
  napi_value js_pin;
  NAPI_CALL(cData->env,
            napi_create_string_utf8(cData->env, pinData, pinSize, &js_pin),
            THROW_BODY(cData->env, ));
  NAPI_CALL(cData->env,
            napi_call_function(cData->env, js_cb, js_cb, 1, &js_pin, nullptr),
            THROW_BODY(cData->env, ));
}

#define PIN_CALLBACK_BINDING(env, info, slug)                           \
  J2C_DECLARE_ARGUMENTS((env), (info), 1);                              \
  J2C_VALIDATE_VALUE_TYPE_THROW((env), arguments[0], napi_function,     \
                                "callback");                            \
  auto js_callback = std::unique_ptr<JSCallback>(new JSCallback);       \
  OCStackResult result = slug(default_##slug, js_callback.get());       \
  if (result == OC_STACK_OK) {                                          \
    HELPER_CALL_THROW((env), (*js_callback).Init((env), arguments[0])); \
    if (slug##_data != nullptr) {                                       \
      JSCallback::Destroy((env), slug##_data);                          \
    }                                                                   \
    slug##_data = js_callback.release();                                \
  }                                                                     \
  C2J_SET_RETURN_VALUE((env), (info), double, ((double)result))

napi_value bind_SetDisplayPinWithContextCB(napi_env env,
                                           napi_callback_info info) {
  PIN_CALLBACK_BINDING(env, info, SetDisplayPinWithContextCB);
}

napi_value bind_SetInputPinWithContextCB(napi_env env,
                                         napi_callback_info info) {
  PIN_CALLBACK_BINDING(env, info, SetInputPinWithContextCB);
}

napi_value bind_SetClosePinDisplayCB(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  J2C_VALIDATE_VALUE_TYPE_THROW(env, arguments[0], napi_function, "callback");
  auto js_callback = std::unique_ptr<JSCallback>(new JSCallback);
  SetClosePinDisplayCB(default_SetClosePinDisplayCB);
  HELPER_CALL_THROW((env), (*js_callback).Init((env), arguments[0]));
  if (SetClosePinDisplayCB_data != nullptr) {
    JSCallback::Destroy((env), SetClosePinDisplayCB_data);
  }
  SetClosePinDisplayCB_data = js_callback.release();
  return nullptr;
}

napi_value bind_UnsetInputPinWithContextCB(napi_env env,
                                           napi_callback_info info) {
  UnsetInputPinWithContextCB();
  JSCallback::Destroy(env, SetInputPinWithContextCB_data);
  SetInputPinWithContextCB_data = nullptr;
  return nullptr;
}

napi_value bind_UnsetDisplayPinWithContextCB(napi_env env,
                                             napi_callback_info info) {
  UnsetDisplayPinWithContextCB();
  JSCallback::Destroy(env, SetDisplayPinWithContextCB_data);
  SetDisplayPinWithContextCB_data = nullptr;
  return nullptr;
}

napi_value bind_GeneratePin(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 1);
  bool is_buffer;
  NAPI_CALL(env, napi_is_buffer(env, arguments[0], &is_buffer));
  JS_ASSERT(is_buffer, "argument is not a Buffer");
  void* buffer_data;
  size_t buffer_length;
  NAPI_CALL(env, napi_get_buffer_info(env, arguments[0], &buffer_data,
                                      &buffer_length));
  char* destination = static_cast<char*>(buffer_data);
  C2J_SET_RETURN_VALUE(env, info, double,
                       (double)GeneratePin(destination, buffer_length));
}

napi_value bind_SetRandomPinPolicy(napi_env env, napi_callback_info info) {
  J2C_DECLARE_ARGUMENTS(env, info, 2);
  J2C_DECLARE_VALUE_JS_THROW(size_t, pinSize, env, arguments[0], napi_number,
                             "PIN size", uint32, uint32_t);
  J2C_DECLARE_VALUE_JS_THROW(OicSecPinType_t, pinType, env, arguments[1],
                             napi_number, "PIN type", uint32, uint32_t);
  C2J_SET_RETURN_VALUE(env, info, double,
                       (double)SetRandomPinPolicy(pinSize, pinType));
}
