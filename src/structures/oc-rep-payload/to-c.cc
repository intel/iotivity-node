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

#include "to-c.h"
extern "C" {
#include <ocpayload.h>
}

class FlatArray {
  std::string flatten(napi_env env, napi_value array);
  std::string fill(napi_env env, napi_value array, int *index);
  void *data;
  size_t dimensions[MAX_REP_ARRAY_DEPTH];
  OCRepPayloadPropType arrayType;
  size_t totalElements;
  bool typeEstablished;

 public:
  FlatArray();
  ~FlatArray();
  std::string copyToPayload(OCRepPayload *payload, const char *propName);
  std::string from(napi_env env, napi_value source);
};

FlatArray::FlatArray()
    : data(nullptr), dimensions(), totalElements(0), typeEstablished(false) {}
FlatArray::~FlatArray() {
  size_t index;
  if (data) {
    for (index = 0; index < totalElements; index++) {
      switch (arrayType) {
        case OCREP_PROP_STRING:
        case OCREP_PROP_BYTE_STRING:
          free(((char **)data)[index]);
          break;

        case OCREP_PROP_OBJECT:
          OCRepPayloadDestroy(((OCRepPayload **)data)[index]);
          break;

        default:
          index = totalElements;
          break;
      }
    }
    free(data);
  }
}

// Fill the array
// p_index contains the index into flatArray where the next item will land. It
// is passed by reference into the recursive instances so that it advances
// monotonically.
std::string FlatArray::fill(napi_env env, napi_value array, int *p_index) {
  bool isArray;
  uint32_t length;
  napi_value member;
  NAPI_CALL_RETURN(napi_get_array_length(env, array, &length));

  for (uint32_t localIndex = 0; localIndex < length; localIndex++) {
    NAPI_CALL_RETURN(napi_get_element(env, array, localIndex, &member));
    NAPI_CALL_RETURN(napi_is_array(env, member, &isArray));
    if (isArray) {
      HELPER_CALL_RETURN(fill(env, member, p_index));
    } else {
      switch (arrayType) {
        case OCREP_PROP_INT:
          NAPI_CALL_RETURN(napi_get_value_int64(
              env, member, &(((int64_t *)data)[(*p_index)++])));
          break;

        case OCREP_PROP_DOUBLE:
          NAPI_CALL_RETURN(napi_get_value_double(
              env, member, &(((double *)data)[(*p_index)++])));
          break;

        case OCREP_PROP_BOOL:
          NAPI_CALL_RETURN(napi_get_value_bool(
              env, member, &(((bool *)data)[(*p_index)++])));
          break;

        case OCREP_PROP_STRING:
          J2C_ASSIGN_STRING_JS_RETURN(env, (((char **)data)[(*p_index)++]),
                                      member, "string member");
          break;

        case OCREP_PROP_OBJECT:
          HELPER_CALL_RETURN(c_OCRepPayload(
              env, member, &(((OCRepPayload **)data)[(*p_index)++])));
          break;

        // The validation should prevent these from occurring
        case OCREP_PROP_NULL:
        case OCREP_PROP_ARRAY:
        default:
          break;
      }
    }
  }

  return std::string();
}

std::string FlatArray::flatten(napi_env env, napi_value array) {
  totalElements = 1;
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

  data = malloc(neededAmount);
  if (!data) {
    return LOCAL_MESSAGE("Not enough memory for flattening rep payload array");
  }

  memset(data, 0, neededAmount);

  int index = 0;
  HELPER_CALL_RETURN(fill(env, array, &index));
  return std::string();
}

// Convert the type of the Javascript value to OCRepPayloadPropType, but only
// for those types that may appear in arrays. Throw an exception otherwise.
static std::string jsTypeToOCArrayType(napi_env env, napi_value value,
                                       OCRepPayloadPropType *p_type) {
  DECLARE_VALUE_TYPE_RETURN(valueType, env, value);

  if (valueType == napi_number) {
    double doubleValue;
    NAPI_CALL_RETURN(napi_get_value_double(env, value, &doubleValue));
    *p_type = ((doubleValue == ((int64_t)doubleValue)) ? OCREP_PROP_INT
                                                       : OCREP_PROP_DOUBLE);
  } else if (valueType == napi_boolean) {
    *p_type = OCREP_PROP_BOOL;
  } else if (valueType == napi_string) {
    *p_type = OCREP_PROP_STRING;
  } else if (valueType == napi_object) {
    bool typecheck;
    NAPI_CALL_RETURN(napi_is_buffer(env, value, &typecheck));
    if (typecheck) {
      *p_type = OCREP_PROP_BYTE_STRING;
    } else {
      *p_type = OCREP_PROP_OBJECT;
    }
  } else {
    return LOCAL_MESSAGE("Value type not allowed in rep payload");
  }

  return std::string();
}

#define SET_ARRAY(setter, payload, name, typeName, description)              \
  if (!OCRepPayloadSet##setter##Array((payload), (name),                     \
                                      (const typeName *)data, dimensions)) { \
    return LOCAL_MESSAGE("Failed to set " description " array property");    \
  }                                                                          \
  break;

std::string FlatArray::copyToPayload(OCRepPayload *payload, const char *name) {
  switch (arrayType) {
    case OCREP_PROP_INT:
      SET_ARRAY(Int, payload, name, int64_t, "integer");

    case OCREP_PROP_DOUBLE:
      SET_ARRAY(Double, payload, name, double, "double");

    case OCREP_PROP_BOOL:
      SET_ARRAY(Bool, payload, name, bool, "boolean");

    case OCREP_PROP_STRING:
      SET_ARRAY(String, payload, name, char *, "string");

    case OCREP_PROP_OBJECT:
      SET_ARRAY(PropObject, payload, name, OCRepPayload *, "object");

    default:
      break;
  }

  return std::string();
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
static std::string validateArray(napi_env env, napi_value array,
                                 bool *p_typeEstablished,
                                 OCRepPayloadPropType *p_arrayType,
                                 size_t dimensions[MAX_REP_ARRAY_DEPTH],
                                 int index) {
  if (index >= MAX_REP_ARRAY_DEPTH) {
    return LOCAL_MESSAGE("Rep payload array has too many dimensions");
  }

  uint32_t length;
  NAPI_CALL_RETURN(napi_get_array_length(env, array, &length));

  if (length > 0) {
    napi_value arrayValue;
    NAPI_CALL_RETURN(napi_get_element(env, array, 0, &arrayValue));
    bool isArray;
    NAPI_CALL_RETURN(napi_is_array(env, arrayValue, &isArray));

    // The first item is an array
    if (isArray) {
      // Establish the length of the first child array
      uint32_t child_length;
      NAPI_CALL_RETURN(napi_get_array_length(env, arrayValue, &child_length));

      // Examine all the children
      for (uint32_t arrayIndex = 0; arrayIndex < length; arrayIndex++) {
        NAPI_CALL_RETURN(napi_get_element(env, array, arrayIndex, &arrayValue));
        NAPI_CALL_RETURN(napi_is_array(env, arrayValue, &isArray));

        // One of the children is not an array
        if (!isArray) {
          return LOCAL_MESSAGE("Rep payload array is heterogeneous");
        }

        bool child_established = false;
        OCRepPayloadPropType child_type;
        uint32_t memberLength;
        NAPI_CALL_RETURN(napi_get_array_length(env, arrayValue, &memberLength));
        if (memberLength != child_length) {
          return LOCAL_MESSAGE(
              "Rep payload array contains child arrays of different lengths");
        }
        HELPER_CALL_RETURN(validateArray(env, arrayValue, &child_established,
                                         &child_type, dimensions, index + 1));

        // Reconcile array types
        if (*p_typeEstablished) {
          if (!child_established || child_type != (*p_arrayType)) {
            return LOCAL_MESSAGE("Rep payload array is heterogeneous");
          }
        } else {
          if (child_established) {
            *p_typeEstablished = true;
            (*p_arrayType) = child_type;
          }
        }
      }

      // The first item is not an array
    } else {
      OCRepPayloadPropType valueType;

      // Establish the type of the first item
      HELPER_CALL_RETURN(jsTypeToOCArrayType(env, arrayValue, &valueType));

      if (*p_typeEstablished) {
        if (valueType != (*p_arrayType)) {
          return LOCAL_MESSAGE("Rep payload array is heterogeneous");
        }
      } else {
        *p_typeEstablished = true;
        *p_arrayType = valueType;
      }

      // Make sure the rest of the items are of the same type
      for (size_t arrayIndex = 1; arrayIndex < length; arrayIndex++) {
        NAPI_CALL_RETURN(napi_get_element(env, array, arrayIndex, &arrayValue));
        HELPER_CALL_RETURN(jsTypeToOCArrayType(env, arrayValue, &valueType));
        if (valueType != (*p_arrayType)) {
          return LOCAL_MESSAGE("Rep payload array is heterogeneous");
        }
      }
    }
  }

  dimensions[index] = length;
  return std::string();
}

std::string FlatArray::from(napi_env env, napi_value array) {
  HELPER_CALL_RETURN(
      validateArray(env, array, &typeEstablished, &arrayType, dimensions, 0));

  if (dimensions[0] > 0) {
    HELPER_CALL_RETURN(flatten(env, array));
  }

  return std::string();
}

static std::string addStrings(napi_env env, napi_value source,
                              OCRepPayload *destination, const char *name,
                              bool (*api)(OCRepPayload *, const char *)) {
  J2C_DECLARE_PROPERTY_JS_RETURN(ll, env, source, name);
  DECLARE_VALUE_TYPE_RETURN(llType, env, ll);

  if (!(llType == napi_undefined || llType == napi_null)) {
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

  return std::string();
}

std::string c_OCRepPayload(napi_env env, napi_value source,
                           OCRepPayload **destination) {
  std::unique_ptr<OCRepPayload, void (*)(OCRepPayload *)> payload(
      OCRepPayloadCreate(), OCRepPayloadDestroy);

  // reppayload.uri
  J2C_DECLARE_PROPERTY_JS_RETURN(jsUri, env, source, "uri");
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

  J2C_DECLARE_PROPERTY_JS_RETURN(jsValues, env, source, "values");
  DECLARE_VALUE_TYPE_RETURN(jsValuesType, env, jsValues);
  if (!(jsValuesType == napi_null || jsValuesType == napi_undefined)) {
    J2C_VALIDATE_VALUE_TYPE_RETURN(env, jsValues, napi_object,
                                   "payload.values");
    napi_value propertyNames;
    std::unique_ptr<char> propNameTracker;
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
    std::unique_ptr<char> strValueTracker;
    NAPI_CALL_RETURN(napi_get_propertynames(env, jsValues, &propertyNames));

    // We can assume propertyNames is an array
    NAPI_CALL_RETURN(napi_get_array_length(env, propertyNames, &length));
    for (index = 0; index < length; index++) {
      NAPI_CALL_RETURN(napi_get_element(env, propertyNames, index, &jsKey));
      msg = std::string("payload.values item[") + std::to_string(index) + "]";
      J2C_GET_STRING_JS_RETURN(env, propName, jsKey, false, msg);
      propNameTracker.reset(propName);
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
        J2C_ASSIGN_STRING_JS_RETURN(env, strValue, jsValue, msg);
        strValueTracker.reset(strValue);
        if (OCRepPayloadSetPropStringAsOwner(payload.get(), propName,
                                             strValue)) {
          strValueTracker.release();
        } else {
          return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
        }
      } else if (jsValueType == napi_object) {
        OCRepPayload *childPayload;
        bool isArray;
        NAPI_CALL_RETURN(napi_is_array(env, jsValue, &isArray));
        if (isArray) {
          FlatArray flatArray;
          HELPER_CALL_RETURN(flatArray.from(env, jsValue));
          HELPER_CALL_RETURN(flatArray.copyToPayload(payload.get(), propName));

          // TODO: What is this? Can it go?
          // String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());

        } else {
          HELPER_CALL_RETURN(c_OCRepPayload(env, jsValue, &childPayload));
          std::unique_ptr<OCRepPayload, void (*)(OCRepPayload *)>
              trackChildPayload(childPayload, OCRepPayloadDestroy);
          if (OCRepPayloadSetPropObjectAsOwner(payload.get(), propName,
                                               childPayload)) {
            trackChildPayload.release();
          } else {
            return LOCAL_MESSAGE(std::string("Failed to set ") + msg);
          }
        }
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
