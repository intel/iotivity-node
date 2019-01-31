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

#ifndef __IOTIVITY_NODE_HANDLES_H__
#define __IOTIVITY_NODE_HANDLES_H__

#include <map>
#include <memory>
#include "common.h"

napi_value JSHandle_constructor(napi_env env, napi_callback_info info);

class JSCallback {
 public:
  napi_ref callback;
  napi_env env;
  JSCallback() : callback(nullptr), env(nullptr) {}
  std::string Init(napi_env _env, napi_value _callback) {
    if (_callback) {
      NAPI_CALL_RETURN(env,
                       napi_create_reference(_env, _callback, 1, &callback));
    }
    env = _env;
    return std::string();
  }
  static std::string Destroy(napi_env env, JSCallback *data) {
    if (data != nullptr && data->callback) {
      NAPI_CALL_RETURN(env, napi_delete_reference(env, data->callback));
    }
    return std::string();
  }
};

template <class jsType, typename T>
class JSHandle : public JSCallback {
 public:
  T data;
  napi_ref self;

  JSHandle() : JSCallback(), data(nullptr), self(nullptr) {}

  std::string Init(napi_env env, napi_value _callback, napi_value _self) {
    HELPER_CALL_RETURN(JSCallback::Init(env, _callback));
    if (_self) {
      NAPI_CALL_RETURN(env, napi_create_reference(env, _self, 1, &self));
    }
    return std::string();
  }

  static std::string New(napi_env _env, napi_value *jsValue, jsType **cData) {
    napi_value theConstructor;
    HELPER_CALL_RETURN(InitClass(_env, &theConstructor));
    NAPI_CALL_RETURN(
        _env, napi_new_instance(_env, theConstructor, 0, nullptr, jsValue));
    auto nativeData = std::unique_ptr<jsType>(new jsType);
    NAPI_CALL_RETURN(_env, napi_wrap(_env, *jsValue, nativeData.get(), nullptr,
                                     nullptr, nullptr));
    *cData = nativeData.release();
    (*cData)->env = _env;
    return std::string();
  }

  static std::string Get(napi_env env, napi_value jsValue, jsType **cData) {
    J2C_VALIDATE_VALUE_TYPE_RETURN(env, jsValue, napi_object, "Handle");

    napi_value jsConstructor;
    HELPER_CALL_RETURN(InitClass(env, &jsConstructor));
    bool isInstanceOf;
    NAPI_CALL_RETURN(
        env, napi_instanceof(env, jsValue, jsConstructor, &isInstanceOf));
    if (!isInstanceOf) {
      return LOCAL_MESSAGE(std::string("Handle is not an object of type ") +
                           jsType::jsClassName());
    }
    void *nativeDataRaw;
    NAPI_CALL_RETURN(env, napi_unwrap(env, jsValue, &nativeDataRaw));
    *cData = (jsType *)nativeDataRaw;
    return std::string();
  }

  static std::string Destroy(napi_env env, jsType *cData,
                             napi_value jsHandle = nullptr) {
    if (cData != nullptr) {
      JSCallback::Destroy(env, cData);
      if (cData->self) {
        void *data = nullptr;
        if (!jsHandle) {
          NAPI_CALL_RETURN(
              env, napi_get_reference_value(env, cData->self, &jsHandle));
        }
        C2J_SET_PROPERTY_CALL_RETURN(
            env, jsHandle, "stale",
            NAPI_CALL_RETURN(env, napi_get_boolean(env, true, &jsValue)));
        NAPI_CALL_RETURN(env, napi_delete_reference(env, cData->self));
        NAPI_CALL_RETURN(env, napi_remove_wrap(env, jsHandle, &data));
      }
      delete cData;
    }
    return std::string();
  }

  static std::string Destroy(napi_env env, napi_value jsHandle) {
    jsType *cData;
    HELPER_CALL_RETURN(Get(env, jsHandle, &cData));
    HELPER_CALL_RETURN(Destroy(env, cData, jsHandle));
    return std::string();
  }

  static std::string InitClass(napi_env env,
                               napi_value *theConstructor = nullptr) {
    static napi_ref localConstructor = nullptr;
    if (!localConstructor) {
      napi_value constructorValue;
      NAPI_CALL_RETURN(
          env, napi_define_class(env, jsType::jsClassName(), NAPI_AUTO_LENGTH,
                                 JSHandle_constructor, nullptr, 0, nullptr,
                                 &constructorValue));
      NAPI_CALL_RETURN(env, napi_create_reference(env, constructorValue, 1,
                                                  &localConstructor));
    }
    if (theConstructor) {
      NAPI_CALL_RETURN(
          env, napi_get_reference_value(env, localConstructor, theConstructor));
    }
    return std::string();
  }
};

class JSOCDoHandle : public JSHandle<JSOCDoHandle, OCDoHandle> {
 public:
  static const char *jsClassName() { return "OCDoHandle"; }
};

class JSOCRequestHandle : public JSHandle<JSOCRequestHandle, OCRequestHandle> {
 public:
  static const char *jsClassName() { return "OCRequestHandle"; }
};

class JSOCResourceHandle
    : public JSHandle<JSOCResourceHandle, OCResourceHandle> {
  typedef JSHandle<JSOCResourceHandle, OCResourceHandle> super;

 public:
  static const char *jsClassName() { return "OCResourceHandle"; }
  static std::map<OCResourceHandle, napi_ref> handles;
  std::string Init(napi_env env, napi_value _callback, napi_value _self) {
    HELPER_CALL_RETURN(super::Init(env, _callback, _self));
    handles[data] = self;
    return std::string();
  }
  static std::string Destroy(napi_env env, JSOCResourceHandle *cData,
                             napi_value jsHandle = nullptr) {
    OCResourceHandle handle = cData->data;
    HELPER_CALL_RETURN(super::Destroy(env, cData, jsHandle));
    handles.erase(handle);
    return std::string();
  }
};

#endif /* __IOTIVITY_NODE_HANDLES_H__ */
