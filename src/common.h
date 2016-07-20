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

#ifndef __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__
#define __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__

#include <string>
#include <v8.h>
#include <nan.h>

#define VALIDATE_CALLBACK_RETURN_VALUE_TYPE(value, typecheck, message) \
  if (!value->typecheck()) {                                           \
    Nan::ThrowTypeError(                                               \
        (std::string(message) +                                        \
         " callback return value type must satisfy " #typecheck "()")  \
            .c_str());                                                 \
  }

#define VALIDATE_ARGUMENT_COUNT(args, length)                               \
  if ((args).Length() < (length)) {                                         \
    return Nan::ThrowRangeError("Argument count must be exactly " #length); \
  }

#define VALIDATE_ARGUMENT_TYPE(args, index, typecheck)                        \
  if (!(args)[(index)]->typecheck()) {                                        \
    return Nan::ThrowTypeError("Argument " #index " must satisfy " #typecheck \
                               "()");                                         \
  }

#define VALIDATE_VALUE_TYPE(value, typecheck, message, failReturn)          \
  if (!(value)->typecheck()) {                                              \
    Nan::ThrowTypeError(                                                    \
        (std::string(message) + " must satisfy " #typecheck "()").c_str()); \
    return failReturn;                                                      \
  }

#define VALIDATE_VALUE_TYPE_OR_FREE(value, typecheck, message, failReturn,  \
                                    pointer_to_free, free_function)         \
  if (!(value)->typecheck()) {                                              \
    Nan::ThrowTypeError(                                                    \
        (std::string(message) + " must satisfy " #typecheck "()").c_str()); \
    free_function((pointer_to_free));                                       \
    return failReturn;                                                      \
  }

#define VALIDATE_ARGUMENT_TYPE_OR_NULL(args, index, typecheck)                \
  if (!((args)[(index)]->typecheck() || (args)[(index)]->IsNull())) {         \
    return Nan::ThrowTypeError("Argument " #index " must satisfy " #typecheck \
                               "() or IsNull()");                             \
  }

#define SET_STRING_IF_NOT_NULL(destination, source, memberName)     \
  if ((source)->memberName) {                                       \
    Nan::Set((destination), Nan::New(#memberName).ToLocalChecked(), \
             Nan::New((source)->memberName).ToLocalChecked());      \
  }

#define SET_VALUE_ON_OBJECT(destination, type, source, memberName) \
  Nan::Set((destination), Nan::New(#memberName).ToLocalChecked(),  \
           Nan::New<type>((source)->memberName));

#define VALIDATE_AND_ASSIGN_STRING(destination, memberName, source,          \
                                   free_function, failReturn)                \
  do {                                                                       \
    char *resultingValue = 0;                                                \
    Local<String> jsMemberName = Nan::New(#memberName).ToLocalChecked();     \
    if ((source)->Has(jsMemberName)) {                                       \
      Local<Value> memberName =                                              \
          Nan::Get((source), jsMemberName).ToLocalChecked();                 \
      VALIDATE_VALUE_TYPE_OR_FREE(memberName, IsString, #memberName, false,  \
                                  destination, free_function);               \
      resultingValue = strdup((const char *)*String::Utf8Value(memberName)); \
      if (!resultingValue) {                                                 \
        Nan::ThrowError("Failed to allocate memory for " #memberName);       \
        free_function((destination));                                        \
        return failReturn;                                                   \
      }                                                                      \
      (destination)->memberName = resultingValue;                            \
    }                                                                        \
  } while (0)

#define VALIDATE_AND_ASSIGN(destination, memberName, destinationType,         \
                            typecheck, message, failReturn, source, accessor) \
  Local<Value> memberName =                                                   \
      Nan::Get(source, Nan::New(#memberName).ToLocalChecked())                \
          .ToLocalChecked();                                                  \
  VALIDATE_VALUE_TYPE(memberName, typecheck, message "." #memberName,         \
                      failReturn);                                            \
  destination.memberName = (destinationType)memberName->accessor();

#define ADD_STRING_ARRAY(destination, source, memberName)                     \
  do {                                                                        \
    int counter;                                                              \
    OCStringLL *item;                                                         \
    for (item = (source)->memberName, counter = 0; item;                      \
         item = item->next, counter++)                                        \
      ;                                                                       \
    Local<Array> jsArray = Nan::New<Array>(counter);                          \
    for (item = (source)->memberName, counter = 0; item;                      \
         item = item->next, counter++) {                                      \
      Nan::Set(jsArray, counter, Nan::New(item->value).ToLocalChecked());     \
    }                                                                         \
    Nan::Set((destination), Nan::New(#memberName).ToLocalChecked(), jsArray); \
  } while (0)

#endif /* __IOTIVITY_NODE_FUNCTIONS_INTERNAL_H__ */
