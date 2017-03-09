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

#ifndef _IOTIVITY_NODE_OC_SERVER_RESOURCE_UTILS_H_
#define _IOTIVITY_NODE_OC_SERVER_RESOURCE_UTILS_H_

#define DECLARE_HANDLE_DATA(varName, env, source, message)               \
  J2C_VALIDATE_VALUE_TYPE_THROW((env), (source), napi_object, message);  \
  JSOCResourceHandle *varName;                                           \
  HELPER_CALL_THROW((env),                                               \
                    JSOCResourceHandle::Get((env), (source), &varName)); \
  JS_ASSERT(varName, std::string() + message + " is invalid",            \
            THROW_BODY(env, ));

#define FIRST_ARGUMENT_IS_HANDLE(argc)    \
  J2C_DECLARE_ARGUMENTS(env, info, argc); \
  DECLARE_HANDLE_DATA(cData, env, arguments[0], "handle");

#endif /* ndef _IOTIVITY_NODE_OC_SERVER_RESOURCE_UTILS_H_ */
