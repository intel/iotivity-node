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

#include "to-js.h"
#include "../oc-payload-macros.h"

#define CREATE_SINGLE_ITEM(env, destination, source, fieldSuffix)              \
  switch ((source)->type) {                                                    \
    case OCREP_PROP_NULL:                                                      \
      NAPI_CALL_RETURN(napi_get_null(env, &(destination)));                    \
      break;                                                                   \
    case OCREP_PROP_INT:                                                       \
      NAPI_CALL_RETURN(napi_create_number(                                     \
          env, (double)((source)->i##fieldSuffix), &(destination)));           \
      break;                                                                   \
    case OCREP_PROP_DOUBLE:                                                    \
      NAPI_CALL_RETURN(                                                        \
          napi_create_number(env, (source)->d##fieldSuffix, &(destination)));  \
      break;                                                                   \
    case OCREP_PROP_BOOL:                                                      \
      NAPI_CALL_RETURN(                                                        \
          napi_create_boolean(env, (source)->b##fieldSuffix, &(destination))); \
      break;                                                                   \
    case OCREP_PROP_STRING:                                                    \
      if (!((source)->str##fieldSuffix)) {                                     \
        continue;                                                              \
      }                                                                        \
      NAPI_CALL_RETURN(napi_create_string_utf8(                                \
          env, (source)->str##fieldSuffix, strlen((source)->str##fieldSuffix), \
          &(destination)));                                                    \
      break;                                                                   \
    case OCREP_PROP_BYTE_STRING:                                               \
      if ((source)->ocByteStr##fieldSuffix.len == 0 ||                         \
          !((source)->ocByteStr##fieldSuffix.bytes)) {                         \
        continue;                                                              \
      }                                                                        \
      NAPI_CALL_RETURN(napi_create_buffer_copy(                                \
          env, (char *)((source)->ocByteStr##fieldSuffix.bytes),               \
          (source)->ocByteStr##fieldSuffix.len, &(destination)));              \
      break;                                                                   \
    case OCREP_PROP_OBJECT:                                                    \
      NAPI_CALL_RETURN(napi_create_object(env, &(destination)));               \
      HELPER_CALL_RETURN(                                                      \
          js_OCRepPayload(env, (source)->obj##fieldSuffix, (destination)));    \
      break;                                                                   \
    default:                                                                   \
      continue;                                                                \
  }

static std::string js_OCRepPayloadValueArray(napi_env env,
                                             OCRepPayloadValueArray *array,
                                             size_t *sharedDataIndex,
                                             int dimensionIndex,
                                             napi_value *destination) {
  NAPI_CALL_RETURN(napi_create_array_with_length(
      env, array->dimensions[dimensionIndex], destination));
  uint32_t index;
  size_t dataIndex;
  napi_value currentValue = nullptr;

  for (index = 0; index < (uint32_t)(array->dimensions[dimensionIndex]);
       index++, currentValue = nullptr) {
    // If this is not the lowest dimension, fill with arrays
    if (dimensionIndex < MAX_REP_ARRAY_DEPTH - 1 &&
        array->dimensions[dimensionIndex + 1] > 0) {
      HELPER_CALL_RETURN(js_OCRepPayloadValueArray(
          env, array, sharedDataIndex, dimensionIndex + 1, &currentValue));

      // Otherwise this is a leaf dimension so fill with values
    } else {
      dataIndex = (*sharedDataIndex)++;
      CREATE_SINGLE_ITEM(env, currentValue, array, Array[dataIndex]);
    }
    NAPI_CALL_RETURN(napi_set_element(env, *destination, index, currentValue));
  }

  return std::string();
}

std::string js_OCRepPayload(napi_env env, OCRepPayload *payload,
                            napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, uri);
  SET_TYPES_INTERFACES(env, destination, payload, types, interfaces);
  if (payload->values) {
    napi_value values;
    OCRepPayloadValue *current;
    napi_value currentValue;
    NAPI_CALL_RETURN(napi_create_object(env, &values));
    for (current = payload->values; current;
         current = current->next, currentValue = nullptr) {
      if (current->type == OCREP_PROP_ARRAY) {
        size_t dataIndex = 0;
        HELPER_CALL_RETURN(js_OCRepPayloadValueArray(
            env, &(current->arr), &dataIndex, 0, &currentValue));
      } else {
        CREATE_SINGLE_ITEM(env, currentValue, current, );
      }
      C2J_SET_PROPERTY_JS_RETURN(env, destination, current->name, currentValue);
    }
  }

  // ignore "next"
  return std::string();
}
