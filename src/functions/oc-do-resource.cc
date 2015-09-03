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

static void deleteNanCallback(NanCallback *callback) { delete callback; }

// Create an object containing the information from an OCClientResponse
// structure
static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle handle, OCClientResponse *clientResponse) {
  NanScope();

  // Call the JS Callback
  Local<Value> jsCallbackArguments[2] = {js_OCDoHandle(handle),
                                         js_OCClientResponse(clientResponse)};

  Local<Value> returnValue = ((NanCallback *)context)->Call(
      NanGetCurrentContext()->Global(), 2, jsCallbackArguments);

  // Validate value we got back from it
  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCClientResponseHandler");

  // Pass on the value to the C API
  return (OCStackApplicationResult)returnValue->Uint32Value();
}

NAN_METHOD(bind_OCDoResource) {
  NanScope();

  OCDevAddr *destination = 0, destinationToFillIn;
  OCPayload *payload = 0;
  OCHeaderOption *options = 0;
  uint8_t optionCount = 0;
  OCDoHandle handle;
  OCCallbackData data;

  VALIDATE_ARGUMENT_COUNT(args, 8);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 3, IsObject);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 4, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 5, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 6, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 7, IsFunction);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 8, IsArray);

  data.context = (void *)(new NanCallback(Local<Function>::Cast(args[7])));
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)deleteNanCallback;

  if (args[8]->IsArray()) {
    Local<Array> optionArray = Local<Array>::Cast(args[8]);
    size_t length = optionArray->Length();

    if (length > 0) {
      options = (OCHeaderOption *)malloc(length * sizeof(OCHeaderOption));
      if (!options) {
        return NanThrowError(
            "Ran out of memory attempting to allocate header options");
        NanReturnUndefined();
      }
      if (!c_OCHeaderOption(optionArray, options, &optionCount)) {
        NanReturnUndefined();
        free(options);
      }
    }
  }

  // If a destination is given, we only use it if it can be converted to a
  // OCDevAddr structure
  if (args[3]->IsObject()) {
    if (c_OCDevAddr(args[3]->ToObject(), &destinationToFillIn)) {
      destination = &destinationToFillIn;
    } else {
      if (options) {
        free(options);
      }
      NanReturnUndefined();
    }
  }

  // If a payload is given, we only use it if it can be converted to a OCPayload
  // *
  if (args[4]->IsObject()) {
    if (!c_OCPayload(args[4]->ToObject(), &payload)) {
      if (options) {
        free(options);
      }
      NanReturnUndefined();
    }
  }

  Local<Number> returnValue = NanNew<Number>((double)OCDoResource(
      &handle, (OCMethod)args[1]->Uint32Value(),
      (const char *)*String::Utf8Value(args[2]), destination, payload,
      (OCConnectivityType)args[5]->Uint32Value(),
      (OCQualityOfService)args[6]->Uint32Value(), &data, options,
      (uint8_t)args[9]->Uint32Value()));

  if (options) {
    free(options);
  }

  args[0]->ToObject()->Set(NanNew<String>("handle"), js_OCDoHandle(handle));

  NanReturnValue(returnValue);
}
