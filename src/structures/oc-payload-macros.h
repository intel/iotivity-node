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

#ifndef _IOTIVITY_NODE_OC_PAYLOAD_MACROS_H_
#define _IOTIVITY_NODE_OC_PAYLOAD_MACROS_H_

#define C2J_SET_LL_PROPERTY(env, destination, source, name, type, createItem) \
  if ((source)->name) {                                                       \
    C2J_SET_PROPERTY_CALL_RETURN((env), (destination), #name, {               \
      uint32_t index;                                                         \
      type current;                                                           \
      napi_value item;                                                        \
      NAPI_CALL_RETURN((env), napi_create_array((env), &jsValue));            \
      for (index = 0, current = (source)->name; current;                      \
           index++, current = current->next) {                                \
        createItem;                                                           \
        NAPI_CALL_RETURN((env), napi_set_element(env, jsValue, index, item)); \
      }                                                                       \
    });                                                                       \
  }

#define C2J_SET_STRING_LL_PROPERTY(env, destination, source, name) \
  C2J_SET_LL_PROPERTY(                                             \
      (env), (destination), (source), name, OCStringLL *,          \
      NAPI_CALL_RETURN(                                            \
          (env), napi_create_string_utf8((env), current->value,    \
                                         strlen(current->value), &item)));

#define SET_TYPES_INTERFACES(env, destination, source, typeField,        \
                             interfaceField)                             \
  C2J_SET_STRING_LL_PROPERTY((env), (destination), (source), typeField); \
  C2J_SET_STRING_LL_PROPERTY((env), (destination), (source), interfaceField);

#endif /* ndef _IOTIVITY_NODE_OC_PAYLOAD_MACROS_H_ */
