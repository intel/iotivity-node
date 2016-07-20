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

#include <nan.h>
#include "../common.h"
#include "oc-payload.h"
#include "oc-sid.h"
#include "string-primitive.h"
#include "oc-platform-info.h"
#include "oc-device-info.h"
#include "handles.h"

extern "C" {
#include <string.h>
#include <ocpayload.h>
}

using namespace v8;

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

static Local<Array> createPayloadValueArrayRecursively(
    OCRepPayloadValueArray *array, size_t *p_dataIndex, int dimensionIndex) {
  size_t index;
  Local<Array> returnValue = Nan::New<Array>(array->dimensions[dimensionIndex]);

  for (index = 0; index < array->dimensions[dimensionIndex]; index++) {
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

static Local<Array> js_OCRepPayloadValueArray(OCRepPayloadValueArray *array) {
  size_t dataIndex = 0;
  return createPayloadValueArrayRecursively(array, &dataIndex, 0);
}

static Local<Object> js_OCRepPayload(OCRepPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();
  int count, index;
  OCStringLL *current;
  OCRepPayloadValue *value;

  // payload.type
  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  // payload.uri
  SET_STRING_IF_NOT_NULL(returnValue, payload, uri);

  // payload.types
  if (payload->types) {
    // Count the items
    for (current = payload->types, count = 0; current;
         current = current->next, count++)
      ;

    Local<Array> types = Nan::New<Array>(count);
    for (current = payload->types, index = 0; current;
         current = current->next, index++) {
      Nan::Set(types, index,
               current->value
                   ? Nan::New<Value>((Handle<String>)Nan::New(current->value)
                                         .ToLocalChecked())
                   : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()));
    }

    Nan::Set(returnValue, Nan::New("types").ToLocalChecked(), types);
  }

  // payload.interfaces
  if (payload->interfaces) {
    // Count the items
    for (current = payload->interfaces, count = 0; current;
         current = current->next, count++)
      ;

    Local<Array> interfaces = Nan::New<Array>(count);
    for (current = payload->interfaces, index = 0; current;
         current = current->next, index++) {
      Nan::Set(interfaces, index,
               current->value
                   ? Nan::New<Value>((Handle<String>)Nan::New(current->value)
                                         .ToLocalChecked())
                   : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()));
    }

    Nan::Set(returnValue, Nan::New("interfaces").ToLocalChecked(), interfaces);
  }

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

  SET_VALUE_ON_OBJECT(returnValue, Number, payload, bitmap);
  SET_VALUE_ON_OBJECT(returnValue, Boolean, payload, secure);
  SET_VALUE_ON_OBJECT(returnValue, Number, payload, port);

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

static Local<Object> js_OCLinksPayload(OCLinksPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  SET_STRING_IF_NOT_NULL(returnValue, payload, href);
  ADD_STRING_ARRAY(returnValue, payload, rt);
  ADD_STRING_ARRAY(returnValue, payload, itf);
  SET_STRING_IF_NOT_NULL(returnValue, payload, rel);
  Nan::Set(returnValue, Nan::New("obs").ToLocalChecked(),
           Nan::New(payload->obs));
  SET_STRING_IF_NOT_NULL(returnValue, payload, title);
  SET_STRING_IF_NOT_NULL(returnValue, payload, uri);
  ADD_STRING_ARRAY(returnValue, payload, mt);

  return returnValue;
}

static Local<Object> js_OCTagsPayload(OCTagsPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("n").ToLocalChecked(),
           js_OCDeviceInfo(&(payload->n)));

  return returnValue;
}

static Local<Object> js_OCResourceCollectionPayload(
    OCResourceCollectionPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("tags").ToLocalChecked(),
           js_OCTagsPayload(payload->tags));
  ADD_STRUCTURE_ARRAY(returnValue, payload, setLinks, OCLinksPayload);

  return returnValue;
}

static Local<Object> js_OCDiscoveryPayload(OCDiscoveryPayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  Nan::Set(returnValue, Nan::New("type").ToLocalChecked(),
           Nan::New(payload->base.type));

  if (payload->sid) {
    Nan::Set(returnValue, Nan::New("sid").ToLocalChecked(),
             js_SID(payload->sid));
  }

  ADD_STRUCTURE_ARRAY(returnValue, payload, resources, OCResourcePayload);

  return returnValue;
}

static Local<Object> js_OCDevicePayload(OCDevicePayload *payload) {
  Local<Object> returnValue = Nan::New<Object>();

  if (payload->sid) {
    Nan::Set(returnValue, Nan::New("sid").ToLocalChecked(),
             js_SID(payload->sid));
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
  SET_VALUE_ON_OBJECT(returnValue, Number, payload, sequenceNumber);
  SET_VALUE_ON_OBJECT(returnValue, Number, payload, maxAge);
  SET_VALUE_ON_OBJECT(returnValue, Number, payload, trigger);
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
                              payload->payloadSize));
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

  size_t length = array->Length();

  if (length > 0) {
    Local<Value> firstValue = Nan::Get(array, 0).ToLocalChecked();
    if (firstValue->IsArray()) {
      size_t child_length = Local<Array>::Cast(firstValue)->Length();
      for (size_t arrayIndex = 0; arrayIndex < length; arrayIndex++) {
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
                Nan::Get(array, arrayIndex).ToLocalChecked(), &valueType)) {
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
  size_t length = array->Length();

  for (size_t localIndex = 0; localIndex < length; localIndex++) {
    Local<Value> member = Nan::Get(array, localIndex).ToLocalChecked();
    if (member->IsArray()) {
      if (!fillArray(flatArray, p_index, Local<Array>::Cast(member),
                     arrayType)) {
        return false;
      }
    } else {
      switch (arrayType) {
        case OCREP_PROP_INT:
          ((uint64_t *)flatArray)[(*p_index)++] =
              (uint64_t)member->Uint32Value();
          break;

        case OCREP_PROP_DOUBLE:
          ((double *)flatArray)[(*p_index)++] = member->NumberValue();
          break;

        case OCREP_PROP_BOOL:
          ((bool *)flatArray)[(*p_index)++] = member->BooleanValue();
          break;

        case OCREP_PROP_STRING: {
          char *theString;

          if (c_StringNew(member->ToString(), &theString)) {
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
          if (c_OCRepPayload(member->ToObject(), &theObject)) {
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

  int neededAmount =
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
    VALIDATE_VALUE_TYPE_OR_FREE(uri, IsString, "reppayload.uri", false, payload,
                                OCRepPayloadDestroy);
    if (!OCRepPayloadSetUri(payload, (const char *)*String::Utf8Value(uri))) {
      goto fail;
    }
  }

  // reppayload.types
  if (jsPayload->Has(Nan::New("types").ToLocalChecked())) {
    Local<Value> types = Nan::Get(jsPayload, Nan::New("types").ToLocalChecked())
                             .ToLocalChecked();
    VALIDATE_VALUE_TYPE_OR_FREE(types, IsArray, "reppayload.types", false,
                                payload, OCRepPayloadDestroy);
    Local<Array> typesArray = Local<Array>::Cast(types);
    length = typesArray->Length();
    for (index = 0; index < length; index++) {
      Local<Value> singleType = Nan::Get(typesArray, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE_OR_FREE(singleType, IsString, "reppayload.types item",
                                  false, payload, OCRepPayloadDestroy);
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
    VALIDATE_VALUE_TYPE_OR_FREE(interfaces, IsArray, "reppayload.interfaces",
                                false, payload, OCRepPayloadDestroy);
    Local<Array> interfacesArray = Local<Array>::Cast(interfaces);
    length = interfacesArray->Length();
    for (index = 0; index < length; index++) {
      Local<Value> singleInterface =
          Nan::Get(interfacesArray, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE_OR_FREE(singleInterface, IsString,
                                  "reppayload.interfaces item", false, payload,
                                  OCRepPayloadDestroy);
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
    VALIDATE_VALUE_TYPE_OR_FREE(values, IsObject, "reppayload.values", false,
                                payload, OCRepPayloadDestroy);
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
        if (!OCRepPayloadSetPropInt(payload, (const char *)*name,
                                    (int64_t)value->Uint32Value())) {
          Nan::ThrowError("reppayload: Failed to set integer property");
          goto fail;
        }
      } else if (value->IsNumber()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetPropDouble(payload, (const char *)*name,
                                       value->NumberValue())) {
          Nan::ThrowError("reppayload: Failed to set floating point property");
          goto fail;
        }
      } else if (value->IsBoolean()) {
        String::Utf8Value name(Nan::Get(keys, index).ToLocalChecked());
        if (!OCRepPayloadSetPropBool(payload, (const char *)*name,
                                     value->BooleanValue())) {
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

        if (c_OCRepPayload(value->ToObject(), &child_payload)) {
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
    VALIDATE_VALUE_TYPE_OR_FREE(next, IsObject, "reppayload.next", false,
                                payload, OCRepPayloadDestroy);
    OCRepPayload *next_payload = 0;
    if (!c_OCRepPayload(next->ToObject(), &next_payload)) {
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
    VALIDATE_VALUE_TYPE(type, IsUint32, "payload.type", false);

    switch (type->Uint32Value()) {
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
