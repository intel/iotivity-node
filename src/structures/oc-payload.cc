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

#include "oc-payload.h"
#include "oc-platform-info.h"

extern "C" {
#include <ocpayload.h>
#include <string.h>
}

static std::string c_OCRepPayload(napi_env env, napi_value source,
                                  OCRepPayload **destination);
static std::string js_OCRepPayload(napi_env env, OCRepPayload *payload,
                                   napi_value destination);

#define C2J_SET_LL_PROPERTY(env, destination, source, name, type, createItem) \
  if ((source)->name) {                                                       \
    C2J_SET_PROPERTY_CALL_RETURN((env), (destination), #name, {               \
      uint32_t index;                                                         \
      type current;                                                           \
      napi_value item;                                                        \
      NAPI_CALL_RETURN(napi_create_array((env), &jsValue));                   \
      for (index = 0, current = (source)->name; current;                      \
           index++, current = current->next) {                                \
        createItem;                                                           \
        NAPI_CALL_RETURN(napi_set_element(env, jsValue, index, item));        \
      }                                                                       \
    });                                                                       \
  }

#define C2J_SET_STRING_LL_PROPERTY(env, destination, source, name) \
  C2J_SET_LL_PROPERTY(                                             \
      (env), (destination), (source), name, OCStringLL *,          \
      NAPI_CALL_RETURN(napi_create_string_utf8(                    \
          (env), current->value, strlen(current->value), &item)));

#define SET_TYPES_INTERFACES(env, destination, source, typeField,        \
                             interfaceField)                             \
  C2J_SET_STRING_LL_PROPERTY((env), (destination), (source), typeField); \
  C2J_SET_STRING_LL_PROPERTY((env), (destination), (source), interfaceField);

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

static std::string js_OCRepPayload(napi_env env, OCRepPayload *payload,
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

std::string js_OCPresencePayload(napi_env env, OCPresencePayload *payload,
                                 napi_value destination) {
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, sequenceNumber);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, maxAge);
  C2J_SET_NUMBER_MEMBER_RETURN(env, destination, payload, trigger);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, resourceType);
  return std::string();
}

static std::string js_OCResourcePayload(napi_env env,
                                        OCResourcePayload *payload,
                                        napi_value *destination) {
  NAPI_CALL_RETURN(napi_create_object(env, destination));
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *destination, payload, uri);

  SET_TYPES_INTERFACES(env, *destination, payload, types, interfaces);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, bitmap);

  C2J_SET_PROPERTY_RETURN(env, *destination, "secure", boolean,
                          payload->secure);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *destination, payload, port);
  return std::string();
}

static std::string js_OCDiscoveryPayload(napi_env env,
                                         OCDiscoveryPayload *payload,
                                         napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, sid);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, baseURI);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, name);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, uri);

  SET_TYPES_INTERFACES(env, destination, payload, type, iface);

  C2J_SET_LL_PROPERTY(
      env, destination, payload, resources, OCResourcePayload *,
      HELPER_CALL_RETURN(js_OCResourcePayload(env, current, &item)));

  // ignore "next"
  return std::string();
}

static std::string js_OCDevicePayload(napi_env env, OCDevicePayload *payload,
                                      napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, sid);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, deviceName);
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, specVersion);

  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, dataModelVersions);

  SET_TYPES_INTERFACES(env, destination, payload, types, interfaces);

  return std::string();
}

static std::string js_OCPlatformPayload(napi_env env,
                                        OCPlatformPayload *payload,
                                        napi_value destination) {
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, destination, payload, uri);
  C2J_SET_PROPERTY_CALL_RETURN(
      env, destination, "info",
      HELPER_CALL_RETURN(js_OCPlatformInfo(env, &(payload->info), &jsValue)));
  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, rt);
  C2J_SET_STRING_LL_PROPERTY(env, destination, payload, interfaces);
  return std::string();
}

std::string js_OCPayload(napi_env env, OCPayload *payload, napi_value *result) {
  NAPI_CALL_RETURN(napi_create_object(env, result));
  C2J_SET_PROPERTY_RETURN(env, *result, "type", number,
                          ((double)(payload->type)));
  switch (payload->type) {
    case PAYLOAD_TYPE_DISCOVERY:
      HELPER_CALL_RETURN(
          js_OCDiscoveryPayload(env, (OCDiscoveryPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_DEVICE:
      HELPER_CALL_RETURN(
          js_OCDevicePayload(env, (OCDevicePayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_PLATFORM:
      HELPER_CALL_RETURN(
          js_OCPlatformPayload(env, (OCPlatformPayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_PRESENCE:
      HELPER_CALL_RETURN(
          js_OCPresencePayload(env, (OCPresencePayload *)payload, *result));
      break;

    case PAYLOAD_TYPE_REPRESENTATION:
      HELPER_CALL_RETURN(
          js_OCRepPayload(env, (OCRepPayload *)payload, *result));
      break;

    /*
        case PAYLOAD_TYPE_SECURITY:
          return js_OCSecurityPayload((OCSecurityPayload *)payload);
    */

    case PAYLOAD_TYPE_INVALID:
    default:
      break;
  }
  return std::string();
}

static std::string addStrings(napi_env env, napi_value source,
                              OCRepPayload *destination, const char *name,
                              bool (*api)(OCRepPayload *, const char *)) {
  J2C_GET_PROPERTY_JS_RETURN(ll, env, source, name);
  DECLARE_VALUE_TYPE(llType, env, ll, return FAIL_STATUS_RETURN);

  if (llType == napi_undefined || llType == napi_null) {
    return std::string();
  }

  std::string message = std::string("payload.") + name;
  std::string itemMessage;
  std::unique_ptr<char> tracker;
  char *strItem;

  J2C_VALIDATE_IS_ARRAY_RETURN(env, ll, true, message);
  uint32_t length, index;
  NAPI_CALL_RETURN(napi_get_array_length(env, ll, &length));
  for (index = 0; index < length; index++) {
    napi_value item;
    NAPI_CALL_RETURN(napi_get_element(env, ll, index, &item));
    itemMessage = message + "[" + std::to_string(index) + "]";
    J2C_GET_STRING_JS_RETURN(env, strItem, item, false, itemMessage);
    tracker.reset(strItem);
    if (!api(destination, strItem)) {
      return LOCAL_MESSAGE(std::string("Failed to set ") + itemMessage);
    } else {
      tracker.release();
    }
  }
}

static std::string c_OCRepPayload(napi_env env, napi_value source,
                                  OCRepPayload **destination) {
  uint32_t index, length;

  std::unique_ptr<OCRepPayload, void (*)(OCRepPayload *)> payload(
      OCRepPayloadCreate(), OCRepPayloadDestroy);

  // reppayload.uri
  J2C_GET_PROPERTY_JS_RETURN(jsUri, env, source, "uri");
  J2C_GET_STRING_TRACKED_JS_RETURN(uri, env, jsUri, true, "payload.uri");
  if (uri) {
    if (!OCRepPayloadSetUri(payload.get(), uri)) {
      return std::string("Failed to set payload.uri");
    }
  }

  // reppayload.types
  HELPER_CALL_RETURN(addStrings(env, source, payload.get(), "types",
                                OCRepPayloadAddResourceType));

  // reppayload.interfaces
  HELPER_CALL_RETURN(addStrings(env, source, payload.get(), "interfaces",
                                OCRepPayloadAddInterface));

  J2C_GET_PROPERTY_JS_RETURN(jsValues, env, source, "values");
  DECLARE_VALUE_TYPE(jsValuesType, env, jsValues, return FAIL_STATUS_RETURN);
  if (!(jsValuesType == napi_null || jsValuesType == napi_undefined)) {
    J2C_VALIDATE_VALUE_TYPE_RETURN(env, jsValues, napi_object,
                                   "payload.values");
    napi_value propertyNames;
    std::unique_ptr<char> tracker;
    std::string msg;
    char *propName;
    uint32_t index, length;
    napi_value jsKey;
    napi_value jsValue;
    napi_valuetype jsValueType;
    double doubleValue;
    int64_t intValue;
    bool boolValue;
    char *strValue;
    NAPI_CALL_RETURN(napi_get_propertynames(env, jsValues, &propertyNames));

    // We can assume propertyNames is an array
    NAPI_CALL_RETURN(napi_get_array_length(env, propertyNames, &length));
    for (index = 0; index < length; index++) {
      NAPI_CALL_RETURN(napi_get_element(env, propertyNames, index, &jsKey));
      msg = std::string("payload.values item[") + std::to_string(index) + "]";
      J2C_GET_STRING_JS_RETURN(env, propName, jsKey, false, msg);
      tracker.reset(propName);
      J2C_ASSIGN_PROPERTY_JS_RETURN(env, jsValues, propName, &jsValue);
      NAPI_CALL_RETURN(napi_get_type_of_value(env, jsValue, &jsValueType));
      msg = std::string("payload.values[\"") + propName + "\"]";
      if (jsValueType == napi_null) {
        if (!OCRepPayloadSetNull(payload.get(), propName)) {
          return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
        }
      } else if (jsValueType == napi_boolean) {
        NAPI_CALL_RETURN(napi_get_value_bool(env, jsValue, &boolValue));
        if (!OCRepPayloadSetPropBool(payload.get(), propName, boolValue)) {
          return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
        }
      } else if (jsValueType == napi_number) {
        NAPI_CALL_RETURN(napi_get_value_double(env, jsValue, &doubleValue));
        intValue = (int64_t)doubleValue;

        // It's exactly an integer
        if (intValue == doubleValue) {
          if (!OCRepPayloadSetPropInt(payload.get(), propName, intValue)) {
            return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
          }

          // It's a double
        } else {
          if (!OCRepPayloadSetPropDouble(payload.get(), propName,
                                         doubleValue)) {
            return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
          }
        }
      } else if (jsValueType == napi_string) {
        J2C_GET_STRING_JS_RETURN(env, strValue, jsValue, false, msg);
      }
    }
  }
  /*
    // reppayload.values
    if (jsPayload->Has(Nan::New("values").ToLocalChecked())) {
      Local<Value> values =
          Nan::Get(jsPayload, Nan::New("values").ToLocalChecked())
              .ToLocalChecked();
      VALIDATE_VALUE_TYPE(values, IsObject, "reppayload.values", goto fail);
      Local<Object> valuesObject = Local<Object>::Cast(values);
      Local<Array> keys = Nan::GetPropertyNames(valuesObject).ToLocalChecked();
      length = keys->Length();
      for (index = 0; index < length; index++) {
        Local<Value> value =
            Nan::Get(valuesObject,
                     Nan::Get(keys, index).ToLocalChecked()->ToString())
                .ToLocalChecked();
        if (value->IsNull()) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          if (!OCRepPayloadSetNull(payload, (const char *)*name)) {
            Nan::ThrowError("reppayload: Failed to set null property");
            goto fail;
          }
        } else if (value->IsUint32()) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          if (!OCRepPayloadSetPropInt(
                  payload, (const char *)*name,
                  (int64_t)Nan::To<uint32_t>(value).FromJust())) {
            Nan::ThrowError("reppayload: Failed to set integer property");
            goto fail;
          }
        } else if (value->IsNumber()) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          if (!OCRepPayloadSetPropDouble(payload, (const char *)*name,
                                         Nan::To<double>(value).FromJust())) {
            Nan::ThrowError("reppayload: Failed to set floating point
    property");
            goto fail;
          }
        } else if (value->IsBoolean()) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          if (!OCRepPayloadSetPropBool(payload, (const char *)*name,
                                       Nan::To<bool>(value).FromJust())) {
            Nan::ThrowError("reppayload: Failed to set boolean property");
            goto fail;
          }
        } else if (value->IsString()) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          String::Utf8Value stringValue(value);
          if (!OCRepPayloadSetPropString(payload, (const char *)*name,
                                         (const char *)*stringValue)) {
            Nan::ThrowError("reppayload: Failed to set string property");
            goto fail;
          }
        } else if (value->IsArray()) {
          size_t dimensions[MAX_REP_ARRAY_DEPTH] = {0};
          bool typeEstablished = false;
          OCRepPayloadPropType arrayType;
          Local<Array> array = Local<Array>::Cast(value);
          if (!validateRepPayloadArray(array, &typeEstablished, &arrayType,
                                       dimensions, 0)) {
            goto fail;
          }

          if (dimensions[0] > 0) {
            void *flatArray;

            if (!flattenArray(array, &flatArray, dimensions, arrayType)) {
              goto fail;
            }

            String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
            switch (arrayType) {
              case OCREP_PROP_INT:
                if (!OCRepPayloadSetIntArray(payload, (const char *)*name,
                                             (const int64_t *)flatArray,
                                             dimensions)) {
                  Nan::ThrowError(
                      "reppayload: Failed to set integer array property");
                  goto fail;
                }
                break;

              case OCREP_PROP_DOUBLE:
                if (!OCRepPayloadSetDoubleArray(payload, (const char *)*name,
                                                (const double *)flatArray,
                                                dimensions)) {
                  Nan::ThrowError(
                      "reppayload: Failed to set double array property");
                  goto fail;
                }
                break;

              case OCREP_PROP_BOOL:
                if (!OCRepPayloadSetBoolArray(payload, (const char *)*name,
                                              (const bool *)flatArray,
                                              dimensions)) {
                  Nan::ThrowError(
                      "reppayload: Failed to set boolean array property");
                  goto fail;
                }
                break;

              case OCREP_PROP_STRING:
                if (!OCRepPayloadSetStringArray(payload, (const char *)*name,
                                                (const char **)flatArray,
                                                dimensions)) {
                  Nan::ThrowError(
                      "reppayload: Failed to set string array property");
                  goto fail;
                }
                break;

              case OCREP_PROP_OBJECT:
                if (!OCRepPayloadSetPropObjectArray(
                        payload, (const char *)*name,
                        (const OCRepPayload **)flatArray, dimensions)) {
                  Nan::ThrowError(
                      "reppayload: Failed to set object array property");
                  goto fail;
                }
                break;

              default:
                break;
            }
          }
        } else if (value->IsObject()) {
          OCRepPayload *child_payload = 0;

          if (c_OCRepPayload(Nan::To<Object>(value).ToLocalChecked(),
                             &child_payload)) {
            String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
            if (!OCRepPayloadSetPropObjectAsOwner(payload, (const char *)*name,
                                                  child_payload)) {
              Nan::ThrowError("reppayload: Failed to set object property");
              goto fail;
            }
          } else {
            goto fail;
          }
        }
      }
    }

    if (jsPayload->Has(Nan::New("next").ToLocalChecked())) {
      Local<Value> next =
          Nan::Get(jsPayload,
    Nan::New("next").ToLocalChecked()).ToLocalChecked();
      VALIDATE_VALUE_TYPE(next, IsObject, "reppayload.next", goto fail);
      OCRepPayload *next_payload = 0;
      if (!c_OCRepPayload(Nan::To<Object>(next).ToLocalChecked(),
                          &next_payload)) {
        goto fail;
      }
      OCRepPayloadAppend(payload, next_payload);
    }
  */
  *destination = payload.release();
  return std::string();
}

std::string c_OCPayload(napi_env env, napi_value source,
                        OCPayload **destination) {
  DECLARE_VALUE_TYPE(jsType, env, source, return FAIL_STATUS_RETURN);
  if (jsType == napi_null || jsType == napi_undefined) {
    *destination = nullptr;
    return std::string();
  }

  J2C_GET_PROPERTY_JS_RETURN(jsPayloadType, env, source, "type");
  J2C_GET_VALUE_JS_RETURN(OCPayloadType, payloadType, env, jsPayloadType,
                          napi_number, "payload.type", uint32, uint32_t);

  if (payloadType != PAYLOAD_TYPE_REPRESENTATION) {
    return LOCAL_MESSAGE("Non-representation payload not supported");
  }

  HELPER_CALL_RETURN(c_OCRepPayload(env, source, (OCRepPayload **)destination));

  return std::string();
}

/*

static Local<Array> createPayloadValueArrayRecursively(
    OCRepPayloadValueArray *array, size_t *p_dataIndex, int dimensionIndex) {
  uint32_t index;
  Local<Array> returnValue = Nan::New<Array>(array->dimensions[dimensionIndex]);

  for (index = 0; index < (uint32_t)(array->dimensions[dimensionIndex]);
       index++) {
    Nan::Set(
        returnValue, index,
        (dimensionIndex < MAX_REP_ARRAY_DEPTH - 1 &&
         array->dimensions[dimensionIndex + 1] > 0)
            ?

            // Fill with arrays
            Nan::New<Value>((Handle<Array>)createPayloadValueArrayRecursively(
                array, p_dataIndex, dimensionIndex + 1))
            :

            // Fill with data
            (array->type == OCREP_PROP_INT
                 ? Nan::New<Value>((Handle<Int32>)Nan::New(
                       (int32_t)array->iArray[(*p_dataIndex)++]))
                 : array->type == OCREP_PROP_DOUBLE
                       ? Nan::New<Value>((Handle<Number>)Nan::New(
                             array->dArray[(*p_dataIndex)++]))
                       : array->type == OCREP_PROP_BOOL
                             ? Nan::New<Value>((Handle<Boolean>)Nan::New(
                                   array->bArray[(*p_dataIndex)++]))
                             : array->type == OCREP_PROP_STRING
                                   ? stringOrUndefined(
                                         array->strArray[(*p_dataIndex)++])
                                   :

                                   // If the type is none of the above, we
                                   // assume it must be an object
                                   objectOrUndefined(
                                       array->objArray[(*p_dataIndex)++])));
  }

  return returnValue;
}

static bool c_OCRepPayload(Local<Object> jsPayload, OCRepPayload **p_payload);
static Local<Object> js_OCRepPayload(OCRepPayload *payload);

static Local<Value> stringOrUndefined(char *str) {
  return (str ? Nan::New<Value>((Handle<String>)Nan::New(str).ToLocalChecked())
              : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()));
}

static Local<Value> objectOrUndefined(OCRepPayload *payload) {
  return (payload ? Nan::New<Value>((Handle<Object>)js_OCRepPayload(payload))
                  : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()));
}

static Local<Array> js_OCRepPayloadValueArray(OCRepPayloadValueArray *array) {
  size_t dataIndex = 0;
  return createPayloadValueArrayRecursively(array, &dataIndex, 0);
}

static Local<Object> js_OCRepPayload(OCRepPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();
  OCRepPayloadValue *value;

  // payload.type
  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  // payload.uri
  SET_STRING_IF_NOT_NULL(returnValue, payload, uri);

  // payload.types
  ADD_STRING_ARRAY(returnValue, payload, types);

  // payload.interfaces
  ADD_STRING_ARRAY(returnValue, payload, interfaces);

  // payload.values
  if (payload->values) {
    Local<Object> values = Nan::New<Object>();
    for (value = payload->values; value; value = value->next) {
      Nan::Set(
          values, Nan::New(value->name).ToLocalChecked(),
          OCREP_PROP_INT == value->type
              ? Nan::New<Value>((Handle<Int32>)Nan::New((int32_t)value->i))
              : OCREP_PROP_DOUBLE == value->type
                    ? Nan::New<Value>((Handle<Number>)Nan::New(value->d))
                    : OCREP_PROP_BOOL == value->type
                          ? Nan::New<Value>((Handle<Boolean>)Nan::New(value->b))
                          : OCREP_PROP_STRING == value->type
                                ? (value->str
                                       ? Nan::New<Value>(
                                             (Handle<String>)Nan::New(
                                                 value->str)
                                                 .ToLocalChecked())
                                       : Nan::New<Value>((Handle<Primitive>)
                                                             Nan::Undefined()))
                                : OCREP_PROP_OBJECT == value->type
                                      ? (value->obj ? Nan::New<Value>(
                                                          (Handle<Object>)
                                                              js_OCRepPayload(
                                                                  value->obj))
                                                    : Nan::New<Value>(
                                                          (Handle<Primitive>)
                                                              Nan::Undefined()))
                                      : OCREP_PROP_ARRAY == value->type
                                            ? Nan::New<Value>(
                                                  (Handle<Object>)
                                                      js_OCRepPayloadValueArray(
                                                          &(value->arr)))
                                            :

                                            // If value->type is not any of the
                                            // above, then we assume it must be
                                            // OCREP_PROP_NULL
                                            Nan::New<Value>((
                                                Handle<Primitive>)Nan::Null()));
    }
    Nan::Set(returnValue, Nan::New("values").ToLocalChecked(), values);
  }

  // payload.next
  if (payload->next) {
    Nan::Set(returnValue, Nan::New("next").ToLocalChecked(),
             js_OCRepPayload(payload->next));
  }

  return returnValue;
}

static Local<Object> js_OCResourcePayload(OCResourcePayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  // payload.uri
  SET_STRING_IF_NOT_NULL(returnValue, payload, uri);

  // payload.types
  ADD_STRING_ARRAY(returnValue, payload, types);

  // payload.interfaces
  ADD_STRING_ARRAY(returnValue, payload, interfaces);

  SET_VALUE_ON_OBJECT(returnValue, payload, bitmap, Number);
  SET_VALUE_ON_OBJECT(returnValue, payload, secure, Boolean);
  SET_VALUE_ON_OBJECT(returnValue, payload, port, Number);

  return returnValue;
}

#define ADD_STRUCTURE_ARRAY(returnValue, payload, memberName, memberType)     \
  do {                                                                        \
    memberType *oneItem = 0;                                                  \
    int counter = 0;                                                          \
    for (counter = 0, oneItem = (payload)->memberName; oneItem;               \
         oneItem = oneItem->next, counter++)                                  \
      ;                                                                       \
    Local<Array> jsArray = Nan::New<Array>(counter);                          \
    for (counter = 0, oneItem = (payload)->memberName; oneItem;               \
         oneItem = oneItem->next, counter++) {                                \
      Nan::Set(jsArray, counter, js_##memberType(oneItem));                   \
    }                                                                         \
    Nan::Set((returnValue), Nan::New(#memberName).ToLocalChecked(), jsArray); \
  } while (0)

static Local<Object> js_OCDiscoveryPayload(OCDiscoveryPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  if (payload->sid) {
    Nan::Set(returnValue, Nan::New("sid").ToLocalChecked(),
             Nan::New(payload->sid).ToLocalChecked());
  }

  ADD_STRUCTURE_ARRAY(returnValue, payload, resources, OCResourcePayload);

  return returnValue;
}

static Local<Object> js_OCDevicePayload(OCDevicePayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  if (payload->sid) {
    Nan::Set(returnValue, Nan::New("sid").ToLocalChecked(),
             Nan::New(payload->sid).ToLocalChecked());
  }

  SET_STRING_IF_NOT_NULL(returnValue, payload, deviceName);
  SET_STRING_IF_NOT_NULL(returnValue, payload, specVersion);

  ADD_STRING_ARRAY(returnValue, payload, dataModelVersions);
  ADD_STRING_ARRAY(returnValue, payload, interfaces);
  ADD_STRING_ARRAY(returnValue, payload, types);

  return returnValue;
}

static Local<Object> js_OCPlatformPayload(OCPlatformPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  SET_STRING_IF_NOT_NULL(returnValue, payload, uri);

  Nan::Set(returnValue, Nan::New("info").ToLocalChecked(),
           js_OCPlatformInfo(&(payload->info)));

  return returnValue;
}

static Local<Object> js_OCPresencePayload(OCPresencePayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));
  SET_VALUE_ON_OBJECT(returnValue, payload, sequenceNumber, Number);
  SET_VALUE_ON_OBJECT(returnValue, payload, maxAge, Number);
  SET_VALUE_ON_OBJECT(returnValue, payload, trigger, Number);
  SET_STRING_IF_NOT_NULL(returnValue, payload, resourceType);

  return returnValue;
}

static Local<Object> js_OCSecurityPayload(OCSecurityPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  if (payload->securityData) {
    Nan::Set(returnValue, Nan::New("securityData").ToLocalChecked(),
             jsArrayFromBytes((unsigned char *)(payload->securityData),
                              (uint32_t)(payload->payloadSize)));
  }

  return returnValue;
}

Local<Value> js_OCPayload(OCPayload *payload) {
  switch (payload->type) {
    case PAYLOAD_TYPE_DISCOVERY:
      return js_OCDiscoveryPayload((OCDiscoveryPayload *)payload);

    case PAYLOAD_TYPE_REPRESENTATION:
      return js_OCRepPayload((OCRepPayload *)payload);

    case PAYLOAD_TYPE_DEVICE:
      return js_OCDevicePayload((OCDevicePayload *)payload);

    case PAYLOAD_TYPE_PLATFORM:
      return js_OCPlatformPayload((OCPlatformPayload *)payload);

    case PAYLOAD_TYPE_PRESENCE:
      return js_OCPresencePayload((OCPresencePayload *)payload);

    case PAYLOAD_TYPE_SECURITY:
      return js_OCSecurityPayload((OCSecurityPayload *)payload);

    case PAYLOAD_TYPE_INVALID:
    default:
      break;
  }
  return Nan::Null();
}

// Convert the type of the Javascript value to OCRepPayloadPropType, but only
// for those types that
// may appear in arrays. Throw an exception otherwise.
static bool jsTypeToOCRepPayloadPropTypeValidForArray(
    Local<Value> value, OCRepPayloadPropType *p_type) {
  // Always check integer before number
  if (value->IsUint32()) {
    *p_type = OCREP_PROP_INT;
    return true;
  } else if (value->IsNumber()) {
    *p_type = OCREP_PROP_DOUBLE;
    return true;
  } else if (value->IsBoolean()) {
    *p_type = OCREP_PROP_BOOL;
    return true;
  } else if (value->IsString()) {
    *p_type = OCREP_PROP_STRING;
    return true;
  } else if (value->IsObject()) {
    *p_type = OCREP_PROP_OBJECT;
    return true;
  }

  Nan::ThrowError("Value type not allowed in rep payload");
  return false;
}

// Validate a multidimensional array of items of a given type
//
// If the array passed in contains arrays, their validity is checked
// recursively, and it is asserted that they all have the same length as the
// first one encountered. Recursions are limited to MAX_REP_ARRAY_DEPTH.
//
// If the array passed in contains primitive types, the type of the array is
// established from the first item, and it is asserted that subsequent items
// have the same type.
//
// If the array is valid, p_typeEstablished, p_arrayType, and dimensions are set
// and the function returns true. Otherwise, it throws an exception and returns
// false.
static bool validateRepPayloadArray(Local<Array> array, bool *p_typeEstablished,
                                    OCRepPayloadPropType *p_arrayType,
                                    size_t dimensions[MAX_REP_ARRAY_DEPTH],
                                    int index) {
  if (index >= MAX_REP_ARRAY_DEPTH) {
    Nan::ThrowError("Rep payload array has too many dimensions");
    return false;
  }

  uint32_t length = array->Length();

  if (length > 0) {
    Local<Value> firstValue = Nan::Get(array, 0).ToLocalChecked();
    if (firstValue->IsArray()) {
      uint32_t child_length = Local<Array>::Cast(firstValue)->Length();
      for (uint32_t arrayIndex = 0; arrayIndex < length; arrayIndex++) {
        Local<Value> member = Nan::Get(array, arrayIndex).ToLocalChecked();
        if (!member->IsArray()) {
          Nan::ThrowError("Rep payload array is heterogeneous");
          return false;
        }

        bool child_established = false;
        OCRepPayloadPropType child_type;
        Local<Array> child_array = Local<Array>::Cast(member);
        if (child_array->Length() != child_length) {
          Nan::ThrowError(
              "Rep payload array contains child arrays of different lengths");
          return false;
        }
        if (!validateRepPayloadArray(child_array, &child_established,
                                     &child_type, dimensions, index + 1)) {
          return false;
        }

        // Reconcile array types
        if (*p_typeEstablished) {
          if (!child_established || child_type != *p_arrayType) {
            Nan::ThrowError("Rep payload array is heterogeneous");
            return false;
          }
        } else {
          if (child_established) {
            *p_typeEstablished = true;
            *p_arrayType = child_type;
          }
        }
      }
    } else {
      OCRepPayloadPropType valueType;

      if (!jsTypeToOCRepPayloadPropTypeValidForArray(firstValue, &valueType)) {
        return false;
      }

      if (*p_typeEstablished) {
        if (valueType != *p_arrayType) {
          Nan::ThrowError("Rep payload array is heterogeneous");
          return false;
        }
      } else {
        *p_typeEstablished = true;
        *p_arrayType = valueType;
      }

      for (size_t arrayIndex = 1; arrayIndex < length; arrayIndex++) {
        if (!jsTypeToOCRepPayloadPropTypeValidForArray(
                Nan::Get(array, (uint32_t)arrayIndex).ToLocalChecked(),
                &valueType)) {
          return false;
        }
        if (valueType != *p_arrayType) {
          Nan::ThrowError("Rep payload array is heterogeneous");
        }
      }
    }
  }

  dimensions[index] = length;
  return true;
}

// Fill the array
// p_index contains the index into flatArray where the next item will land. It
// is passed by reference into the recursive instances so that it advances
// monotonically.
static bool fillArray(void *flatArray, int *p_index, Local<Array> array,
                      OCRepPayloadPropType arrayType) {
  uint32_t length = array->Length();

  for (uint32_t localIndex = 0; localIndex < length; localIndex++) {
    Local<Value> member =
        Nan::Get(array, (uint32_t)localIndex).ToLocalChecked();
    if (member->IsArray()) {
      if (!fillArray(flatArray, p_index, Local<Array>::Cast(member),
                     arrayType)) {
        return false;
      }
    } else {
      switch (arrayType) {
        case OCREP_PROP_INT:
          ((uint64_t *)flatArray)[(*p_index)++] =
              (uint64_t)Nan::To<uint32_t>(member).FromJust();
          break;

        case OCREP_PROP_DOUBLE:
          ((double *)flatArray)[(*p_index)++] =
              Nan::To<double>(member).FromJust();
          break;

        case OCREP_PROP_BOOL:
          ((bool *)flatArray)[(*p_index)++] = Nan::To<bool>(member).FromJust();
          break;

        case OCREP_PROP_STRING: {
          char *theString = strdup((const char *)*(String::Utf8Value(member)));

          if (theString) {
            ((char **)flatArray)[(*p_index)++] = theString;
          } else {
            // If we fail to copy the string, we free all strings allocated so
            // far and we quit.
            for (int freeIndex = 0; freeIndex < (*p_index); freeIndex++) {
              free(((char **)flatArray)[freeIndex]);
            }
            return false;
          }
          break;
        }

        case OCREP_PROP_OBJECT: {
          OCRepPayload *theObject = 0;
          if (c_OCRepPayload(Nan::To<Object>(member).ToLocalChecked(),
                             &theObject)) {
            ((OCRepPayload **)flatArray)[(*p_index)++] = theObject;
          } else {
            // If we fail to create the object, we free all objects allocated
            // so far and we quit.
            for (int freeIndex = 0; freeIndex < (*p_index); freeIndex++) {
              OCRepPayloadDestroy(((OCRepPayload **)flatArray)[freeIndex]);
            }
            return false;
          }
          break;
        }

        // The validation should prevent these from occurring
        case OCREP_PROP_NULL:
        case OCREP_PROP_ARRAY:
        default:
          break;
      }
    }
  }

  return true;
}

static bool flattenArray(Local<Array> array, void **flatArray,
                         size_t dimensions[MAX_REP_ARRAY_DEPTH],
                         OCRepPayloadPropType arrayType) {
  size_t totalElements = 1;
  void *returnValue = 0;

  for (size_t dimensionIndex = 0;
       dimensionIndex < MAX_REP_ARRAY_DEPTH && dimensions[dimensionIndex] > 0;
       dimensionIndex++) {
    totalElements *= dimensions[dimensionIndex];
  }

  size_t neededAmount =
      (arrayType == OCREP_PROP_INT
           ? sizeof(uint64_t)
           : arrayType == OCREP_PROP_DOUBLE
                 ? sizeof(double)
                 : arrayType == OCREP_PROP_BOOL
                       ? sizeof(bool)
                       : arrayType == OCREP_PROP_STRING
                             ? sizeof(char *)
                             : arrayType == OCREP_PROP_OBJECT
                                   ? sizeof(OCRepPayload *)
                                   :

                                   // The validation ensures that the
                                   // array type is always valid
                                   0) *
      totalElements;

  returnValue = malloc(neededAmount);

  if (!returnValue) {
    Nan::ThrowError("Not enough memory for flattening rep payload array");
    return false;
  }

  memset(returnValue, 0, neededAmount);

  int index = 0;
  if (!fillArray(returnValue, &index, array, arrayType)) {
    free(returnValue);
    return false;
  }

  *flatArray = returnValue;
  return true;
}

static bool c_OCRepPayload(Local<Object> jsPayload, OCRepPayload **p_payload) {
  uint32_t index, length;
  OCRepPayload *payload = OCRepPayloadCreate();

  // reppayload.uri
  if (jsPayload->Has(Nan::New("uri").ToLocalChecked())) {
    Local<Value> uri =
        Nan::Get(jsPayload, Nan::New("uri").ToLocalChecked()).ToLocalChecked();
    VALIDATE_VALUE_TYPE(uri, IsString, "reppayload.uri", goto fail);
    if (!OCRepPayloadSetUri(payload, (const char *)*String::Utf8Value(uri))) {
      goto fail;
    }
  }

  // reppayload.types
  if (jsPayload->Has(Nan::New("types").ToLocalChecked())) {
    Local<Value> types = Nan::Get(jsPayload, Nan::New("types").ToLocalChecked())
                             .ToLocalChecked();
    VALIDATE_VALUE_TYPE(types, IsArray, "reppayload.types", goto fail);
    Local<Array> typesArray = Local<Array>::Cast(types);
    length = typesArray->Length();
    for (index = 0; index < length; index++) {
      Local<Value> singleType = Nan::Get(typesArray, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE(singleType, IsString, "reppayload.types item",
                          goto fail);
      if (!OCRepPayloadAddResourceType(
              payload, (const char *)*String::Utf8Value(singleType))) {
        goto fail;
      }
    }
  }

  // reppayload.interfaces
  if (jsPayload->Has(Nan::New("interfaces").ToLocalChecked())) {
    Local<Value> interfaces =
        Nan::Get(jsPayload, Nan::New("interfaces").ToLocalChecked())
            .ToLocalChecked();
    VALIDATE_VALUE_TYPE(interfaces, IsArray, "reppayload.interfaces",
                        goto fail);
    Local<Array> interfacesArray = Local<Array>::Cast(interfaces);
    length = interfacesArray->Length();
    for (index = 0; index < length; index++) {
      Local<Value> singleInterface =
          Nan::Get(interfacesArray, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE(singleInterface, IsString,
                          "reppayload.interfaces item", goto fail);
      if (!OCRepPayloadAddInterface(
              payload, (const char *)*String::Utf8Value(singleInterface))) {
        goto fail;
      }
    }
  }

  // reppayload.values
  if (jsPayload->Has(Nan::New("values").ToLocalChecked())) {
    Local<Value> values =
        Nan::Get(jsPayload, Nan::New("values").ToLocalChecked())
            .ToLocalChecked();
    VALIDATE_VALUE_TYPE(values, IsObject, "reppayload.values", goto fail);
    Local<Object> valuesObject = Local<Object>::Cast(values);
    Local<Array> keys = Nan::GetPropertyNames(valuesObject).ToLocalChecked();
    length = keys->Length();
    for (index = 0; index < length; index++) {
      Local<Value> value =
          Nan::Get(valuesObject,
                   Nan::Get(keys, index).ToLocalChecked()->ToString())
              .ToLocalChecked();
      if (value->IsNull()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetNull(payload, (const char *)*name)) {
          Nan::ThrowError("reppayload: Failed to set null property");
          goto fail;
        }
      } else if (value->IsUint32()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetPropInt(
                payload, (const char *)*name,
                (int64_t)Nan::To<uint32_t>(value).FromJust())) {
          Nan::ThrowError("reppayload: Failed to set integer property");
          goto fail;
        }
      } else if (value->IsNumber()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetPropDouble(payload, (const char *)*name,
                                       Nan::To<double>(value).FromJust())) {
          Nan::ThrowError("reppayload: Failed to set floating point property");
          goto fail;
        }
      } else if (value->IsBoolean()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetPropBool(payload, (const char *)*name,
                                     Nan::To<bool>(value).FromJust())) {
          Nan::ThrowError("reppayload: Failed to set boolean property");
          goto fail;
        }
      } else if (value->IsString()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        String::Utf8Value stringValue(value);
        if (!OCRepPayloadSetPropString(payload, (const char *)*name,
                                       (const char *)*stringValue)) {
          Nan::ThrowError("reppayload: Failed to set string property");
          goto fail;
        }
      } else if (value->IsArray()) {
        size_t dimensions[MAX_REP_ARRAY_DEPTH] = {0};
        bool typeEstablished = false;
        OCRepPayloadPropType arrayType;
        Local<Array> array = Local<Array>::Cast(value);
        if (!validateRepPayloadArray(array, &typeEstablished, &arrayType,
                                     dimensions, 0)) {
          goto fail;
        }

        if (dimensions[0] > 0) {
          void *flatArray;

          if (!flattenArray(array, &flatArray, dimensions, arrayType)) {
            goto fail;
          }

          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          switch (arrayType) {
            case OCREP_PROP_INT:
              if (!OCRepPayloadSetIntArray(payload, (const char *)*name,
                                           (const int64_t *)flatArray,
                                           dimensions)) {
                Nan::ThrowError(
                    "reppayload: Failed to set integer array property");
                goto fail;
              }
              break;

            case OCREP_PROP_DOUBLE:
              if (!OCRepPayloadSetDoubleArray(payload, (const char *)*name,
                                              (const double *)flatArray,
                                              dimensions)) {
                Nan::ThrowError(
                    "reppayload: Failed to set double array property");
                goto fail;
              }
              break;

            case OCREP_PROP_BOOL:
              if (!OCRepPayloadSetBoolArray(payload, (const char *)*name,
                                            (const bool *)flatArray,
                                            dimensions)) {
                Nan::ThrowError(
                    "reppayload: Failed to set boolean array property");
                goto fail;
              }
              break;

            case OCREP_PROP_STRING:
              if (!OCRepPayloadSetStringArray(payload, (const char *)*name,
                                              (const char **)flatArray,
                                              dimensions)) {
                Nan::ThrowError(
                    "reppayload: Failed to set string array property");
                goto fail;
              }
              break;

            case OCREP_PROP_OBJECT:
              if (!OCRepPayloadSetPropObjectArray(
                      payload, (const char *)*name,
                      (const OCRepPayload **)flatArray, dimensions)) {
                Nan::ThrowError(
                    "reppayload: Failed to set object array property");
                goto fail;
              }
              break;

            default:
              break;
          }
        }
      } else if (value->IsObject()) {
        OCRepPayload *child_payload = 0;

        if (c_OCRepPayload(Nan::To<Object>(value).ToLocalChecked(),
                           &child_payload)) {
          String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
          if (!OCRepPayloadSetPropObjectAsOwner(payload, (const char *)*name,
                                                child_payload)) {
            Nan::ThrowError("reppayload: Failed to set object property");
            goto fail;
          }
        } else {
          goto fail;
        }
      }
    }
  }

  if (jsPayload->Has(Nan::New("next").ToLocalChecked())) {
    Local<Value> next =
        Nan::Get(jsPayload, Nan::New("next").ToLocalChecked()).ToLocalChecked();
    VALIDATE_VALUE_TYPE(next, IsObject, "reppayload.next", goto fail);
    OCRepPayload *next_payload = 0;
    if (!c_OCRepPayload(Nan::To<Object>(next).ToLocalChecked(),
                        &next_payload)) {
      goto fail;
    }
    OCRepPayloadAppend(payload, next_payload);
  }

  (*p_payload) = payload;
  return true;

fail:
  OCRepPayloadDestroy(payload);
  return false;
}

bool c_OCPayload(Local<Object> jsPayload, OCPayload **p_payload) {
  if (!jsPayload->IsNull()) {
    Local<Value> type =
        Nan::Get(jsPayload, Nan::New("type").ToLocalChecked()).ToLocalChecked();
    VALIDATE_VALUE_TYPE(type, IsUint32, "payload.type", return false);

    switch (Nan::To<uint32_t>(type).FromJust()) {
      case PAYLOAD_TYPE_REPRESENTATION:
        return c_OCRepPayload(jsPayload, (OCRepPayload **)p_payload);

      default:
        Nan::ThrowError("Support for this payload type not implemented");
        return false;
    }
  } else {
    *p_payload = 0;
  }
  return true;
}
*/
