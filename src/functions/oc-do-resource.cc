#include "oc-do-resource.h"
#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-header-option-array.h"
#include "../structures/oc-client-response.h"
#include "../structures/oc-dev-addr.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
#include <ocpayload.h>
}

using namespace v8;
using namespace node;

static void deleteNanCallback(Nan::Callback *callback) { delete callback; }

// Create an object containing the information from an OCClientResponse
// structure
static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle handle, OCClientResponse *clientResponse) {
  // Call the JS Callback
  Local<Value> jsCallbackArguments[2] = {js_OCDoHandle(handle),
                                         js_OCClientResponse(clientResponse)};

  Local<Value> returnValue =
      ((Nan::Callback *)context)->Call(2, jsCallbackArguments);

  // Validate value we got back from it
  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCClientResponseHandler");

  // Pass on the value to the C API
  return (OCStackApplicationResult)returnValue->Uint32Value();
}

NAN_METHOD(bind_OCDoResource) {
  VALIDATE_ARGUMENT_COUNT(info, 8);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 3, IsObject);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 4, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 5, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 6, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 7, IsFunction);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 8, IsArray);

  OCDevAddr *destination = 0, destinationToFillIn;
  OCPayload *payload = 0;
  OCHeaderOption *options = 0;
  uint8_t optionCount = 0;
  OCDoHandle handle;
  OCCallbackData data;

  data.context = (void *)(new Nan::Callback(Local<Function>::Cast(info[7])));
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)deleteNanCallback;

  if (info[8]->IsArray()) {
    Local<Array> optionArray = Local<Array>::Cast(info[8]);
    size_t length = optionArray->Length();

    if (length > 0) {
      options = (OCHeaderOption *)malloc(length * sizeof(OCHeaderOption));
      if (!options) {
        Nan::ThrowError(
            "Ran out of memory attempting to allocate header options");
        return;
      }
      if (!c_OCHeaderOption(optionArray, options, &optionCount)) {
        free(options);
        return;
      }
    }
  }

  // If a destination is given, we only use it if it can be converted to a
  // OCDevAddr structure
  if (info[3]->IsObject()) {
    if (c_OCDevAddr(info[3]->ToObject(), &destinationToFillIn)) {
      destination = &destinationToFillIn;
    } else {
      free(options);
      return;
    }
  }

  // If a payload is given, we only use it if it can be converted to a OCPayload
  // *
  if (info[4]->IsObject()) {
    if (!c_OCPayload(info[4]->ToObject(), &payload)) {
      free(options);
      return;
    }
  }

  Local<Number> returnValue = Nan::New(
      OCDoResource(&handle, (OCMethod)info[1]->Uint32Value(),
                   (const char *)*String::Utf8Value(info[2]), destination,
                   payload, (OCConnectivityType)info[5]->Uint32Value(),
                   (OCQualityOfService)info[6]->Uint32Value(), &data, options,
                   (uint8_t)info[9]->Uint32Value()));

  free(options);

  // We need not free the payload because it seems iotivity takes ownership.

  info[0]->ToObject()->Set(Nan::New("handle").ToLocalChecked(),
                           js_OCDoHandle(handle));

  info.GetReturnValue().Set(returnValue);
}
