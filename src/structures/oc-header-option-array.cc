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
  Local<Array> returnValue = NanNew<Array>(optionCount);

  for (index = 0; index < optionCount; index++) {
    // options[ index ]
    Local<Object> item = NanNew<Object>();

    SET_VALUE_ON_OBJECT(item, Number, &options[index], protocolID);
    SET_VALUE_ON_OBJECT(item, Number, &options[index], optionID);

    // option[ index ].optionData
    Local<Array> optionData = NanNew<Array>(options[index].optionLength);
    for (dataIndex = 0; dataIndex < options[index].optionLength; dataIndex++) {
      optionData->Set(dataIndex,
                      NanNew<Number>(options[index].optionData[dataIndex]));
    }

    returnValue->Set(index, item);
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
    NanThrowRangeError("Too many header options");
    return false;
  }

  if (length > 0) {
    for (index = 0; index < length; index++) {
      // option[ index ]
      Local<Value> jsOptionsItem = jsOptions->Get(index);
      VALIDATE_VALUE_TYPE(jsOptionsItem, IsObject, "OCHeaderOption array item",
                          false);
      Local<Object> jsOptionsItemObject = Local<Object>::Cast(jsOptionsItem);

      // option[ index ].protocolID
      Local<Value> protocolID =
          jsOptionsItemObject->Get(NanNew<String>("protocolID"));
      VALIDATE_VALUE_TYPE(protocolID, IsNumber,
                          "(OCHeaderOption array item).protocolID", false);
      options[index].protocolID =
          (OCTransportProtocolID)protocolID->Uint32Value();

      // option[ index ].optionID
      Local<Value> optionID =
          jsOptionsItemObject->Get(NanNew<String>("optionID"));
      VALIDATE_VALUE_TYPE(optionID, IsNumber,
                          "(OCHeaderOption array item).optionID", false);
      options[index].optionID = (uint16_t)optionID->Uint32Value();

      // option[ index ].optionData
      Local<Value> optionData =
          jsOptionsItemObject->Get(NanNew<String>("optionData"));
      VALIDATE_VALUE_TYPE(optionData, IsArray,
                          "(OCHeaderOption array item).optionData", false);
      Local<Array> optionDataArray = Local<Array>::Cast(optionData);
      dataLength = optionDataArray->Length();
      if (length > MAX_HEADER_OPTION_DATA_LENGTH) {
        NanThrowRangeError(
            "(OCHeaderOption array item).optionData: Number of JS structure "
            "data bytes exceeds "
            "MAX_HEADER_OPTION_DATA_LENGTH");
        return false;
      }
      for (dataIndex = 0; dataIndex < MAX_HEADER_OPTION_DATA_LENGTH;
           dataIndex++) {
        if (dataIndex < dataLength) {
          Local<Value> optionDataItem = optionDataArray->Get(dataIndex);
          VALIDATE_VALUE_TYPE(optionDataItem, IsNumber,
                              "(OCHeaderOption array item).optionData item",
                              false);
          options[index].optionData[dataIndex] =
              (uint8_t)optionDataItem->Uint32Value();
        } else {
          options[index].optionData[dataIndex] = 0;
        }
      }
    }
  }

  memcpy(p_options, options, MAX_HEADER_OPTIONS * sizeof(OCHeaderOption));
  *p_optionCount = length;
  return true;
}
