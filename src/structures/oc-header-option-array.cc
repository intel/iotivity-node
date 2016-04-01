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
#include "oc-header-option-array.h"
#include "../common.h"

extern "C" {
#include <stdlib.h>
#include <string.h>
}

using namespace v8;

v8::Local<v8::Array> js_OCHeaderOption(OCHeaderOption *options,
                                       uint8_t optionCount) {
  uint8_t index;
  uint16_t dataIndex;
  Local<Array> returnValue = Nan::New<Array>(optionCount);

  for (index = 0; index < optionCount; index++) {
    // options[ index ]
    Local<Object> item = Nan::New<Object>();

    SET_VALUE_ON_OBJECT(item, Number, &options[index], protocolID);
    SET_VALUE_ON_OBJECT(item, Number, &options[index], optionID);

    // option[ index ].optionData
    uint16_t optionLength =
        (options[index].optionLength > MAX_HEADER_OPTION_DATA_LENGTH
             ? MAX_HEADER_OPTION_DATA_LENGTH
             : options[index].optionLength);
    Local<Array> optionData = Nan::New<Array>(optionLength);
    for (dataIndex = 0; dataIndex < optionLength; dataIndex++) {
      Nan::Set(optionData, dataIndex,
               Nan::New(options[index].optionData[dataIndex]));
    }

    Nan::Set(returnValue, index, item);
  }

  return returnValue;
}

bool c_OCHeaderOption(v8::Local<v8::Array> jsOptions, OCHeaderOption *p_options,
                      uint8_t *p_optionCount) {
  uint8_t index, dataIndex;
  uint16_t dataLength;
  uint8_t length = (uint8_t)jsOptions->Length();
  OCHeaderOption options[MAX_HEADER_OPTIONS];

  if (length > MAX_HEADER_OPTIONS) {
    Nan::ThrowRangeError("Too many header options");
    return false;
  }

  if (length > 0) {
    for (index = 0; index < length; index++) {
      // option[ index ]
      Local<Value> item = Nan::Get(jsOptions, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE(item, IsObject, "OCHeaderOption array item", false);
      Local<Object> itemObject = Local<Object>::Cast(item);

      VALIDATE_AND_ASSIGN(options[index], protocolID, OCTransportProtocolID,
                          IsUint32, "(OCHeaderOption array item)", false,
                          itemObject, Uint32Value);
      VALIDATE_AND_ASSIGN(options[index], optionID, uint16_t, IsUint32,
                          "(OCHeaderOption array item)", false, itemObject,
                          Uint32Value);

      // option[ index ].optionData
      Local<Value> optionData =
          Nan::Get(itemObject, Nan::New("optionData").ToLocalChecked())
              .ToLocalChecked();
      VALIDATE_VALUE_TYPE(optionData, IsArray,
                          "(OCHeaderOption array item).optionData", false);
      Local<Array> optionDataArray = Local<Array>::Cast(optionData);
      dataLength = optionDataArray->Length();
      if (dataLength > MAX_HEADER_OPTION_DATA_LENGTH) {
        Nan::ThrowRangeError(
            "(OCHeaderOption array item).optionData: Number of JS structure "
            "data bytes exceeds "
            "MAX_HEADER_OPTION_DATA_LENGTH");
        return false;
      }
      for (dataIndex = 0; dataIndex < MAX_HEADER_OPTION_DATA_LENGTH;
           dataIndex++) {
        if (dataIndex < dataLength) {
          Local<Value> optionDataItem =
              Nan::Get(optionDataArray, dataIndex).ToLocalChecked();
          VALIDATE_VALUE_TYPE(optionDataItem, IsUint32,
                              "(OCHeaderOption array item).optionData item",
                              false);
          options[index].optionData[dataIndex] =
              (uint8_t)optionDataItem->Uint32Value();
        } else {
          options[index].optionData[dataIndex] = 0;
        }
      }
      options[index].optionLength = dataLength;
    }
  }

  memcpy(p_options, options, MAX_HEADER_OPTIONS * sizeof(OCHeaderOption));
  *p_optionCount = length;
  return true;
}
