#include <node_buffer.h>
#include "oc-do-resource.h"
#include "../common.h"
#include "../structures.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

// Create an object containing the information from an OCCLientResponse
// structure
static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle handle, OCClientResponse *clientResponse) {
  // Call the JS Callback
  Local<Value> jsCallbackArguments[2] = {
      NanNewBufferHandle((char *)&handle, sizeof(OCDoHandle)),
      js_OCClientResponse(clientResponse)};
  Local<Value> returnValue = NanMakeCallback(
      NanGetCurrentContext()->Global(),
      NanNew(*(Persistent<Function> *)context), 2, jsCallbackArguments);

  // Validate value we got back from it
  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCClientResponseHandler");

  // Pass on the value to the C API
  return (OCStackApplicationResult)returnValue->Uint32Value();
}

static OCHeaderOption *oc_header_options_new(Handle<Array> array) {
  int index, optionIndex, optionLength;
  int count = array->Length();
  OCHeaderOption *options =
      (OCHeaderOption *)malloc(sizeof(OCHeaderOption) * count);

  if (options) {
    for (index = 0; index < count; index++) {
      options[index].protocolID = (OCTransportProtocolID)array->Get(index)
                                      ->ToObject()
                                      ->Get(NanNew<String>("protocolID"))
                                      ->Uint32Value();
      options[index].optionID = (uint16_t)array->Get(index)
                                    ->ToObject()
                                    ->Get(NanNew<String>("optionID"))
                                    ->Uint32Value();
      options[index].optionLength = (uint16_t)array->Get(index)
                                        ->ToObject()
                                        ->Get(NanNew<String>("optionLength"))
                                        ->Uint32Value();

      Handle<Array> jsOption = Handle<Array>::Cast(
          array->Get(index)->ToObject()->Get(NanNew<String>("optionData")));
      optionLength = jsOption->Length();
      optionLength = (optionLength > MAX_HEADER_OPTION_DATA_LENGTH)
                         ? MAX_HEADER_OPTION_DATA_LENGTH
                         : optionLength;

      for (optionIndex = 0; optionIndex < optionLength; optionIndex++) {
        options[index].optionData[optionIndex] =
            (uint8_t)jsOption->Get(optionIndex)->Uint32Value();
      }
    }
  }

  return options;
}

// Always returns NULL
static OCHeaderOption *oc_header_options_free(OCHeaderOption *options) {
  if (options) {
    free((void *)options);
  }
  return 0;
}

NAN_METHOD(bind_OCDoResource) {
  NanScope();

  OCHeaderOption *options = 0;
  OCDoHandle handle;
  OCCallbackData data;

  VALIDATE_ARGUMENT_COUNT(args, 9);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 3, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 4, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 5, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 6, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 7, IsFunction);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 8, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 9, IsUint32);

  data.context =
      (void *)persistentJSCallback_new(Local<Function>::Cast(args[7]));
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)persistentJSCallback_free;

  if (args[8]->IsArray()) {
    options = oc_header_options_new(Handle<Array>::Cast(args[8]));
  }

  Local<Number> returnValue = NanNew<Number>((double)OCDoResource(
      &handle, (OCMethod)args[1]->Uint32Value(),
      (const char *)*String::Utf8Value(args[2]),
      (const char *)(args[3]->IsString() ? (*String::Utf8Value(args[3])) : 0),
      (const char *)(args[4]->IsString() ? (*String::Utf8Value(args[4])) : 0),
      (OCConnectivityType)args[5]->Uint32Value(),
      (OCQualityOfService)args[6]->Uint32Value(), &data, options,
      (uint8_t)args[9]->Uint32Value()));

  options = oc_header_options_free(options);

  args[0]->ToObject()->Set(
      NanNew<String>("handle"),
      NanNewBufferHandle((const char *)&handle, sizeof(OCDoHandle)));

  NanReturnValue(returnValue);
}
