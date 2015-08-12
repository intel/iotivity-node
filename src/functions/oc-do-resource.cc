#include <node_buffer.h>
#include "oc-do-resource.h"
#include "../common.h"
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

// Create an object containing the information from an OCClientResponse
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

  data.context =
      (void *)persistentJSCallback_new(Local<Function>::Cast(args[7]));
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)persistentJSCallback_free;

  if (args[8]->IsArray()) {
  	if ( !c_OCHeaderOption( Local<Array>::Cast( args[ 8 ] ), &options, &optionCount ) ) {
		NanReturnUndefined();
	}
  }

	// If a destination is given, we only use it if it can be converted to a OCDevAddr structure
	if ( args[ 3 ]->IsObject() ) {
		if ( c_OCDevAddr( args[ 3 ]->ToObject(), &destinationToFillIn ) ) {
			destination = &destinationToFillIn;
		} else {
			NanReturnUndefined();
		}
	}

	// If a payload is given, we only use it if it can be converted to a OCPayload *
	if ( args[ 4 ]->IsObject() ) {
		if ( !c_OCPayload( &payload, args[ 4 ]->ToObject() ) ) {
			NanReturnUndefined();
		}
	}

  Local<Number> returnValue = NanNew<Number>(
      (double)OCDoResource(&handle, (OCMethod)args[1]->Uint32Value(),
                           (const char *)*String::Utf8Value(args[2]),
						   destination, payload,
                           (OCConnectivityType)args[5]->Uint32Value(),
                           (OCQualityOfService)args[6]->Uint32Value(), &data,
                           options, (uint8_t)args[9]->Uint32Value()));

	if ( optionCount > 0 ) {
		free( options );
	}

  args[0]->ToObject()->Set(
      NanNew<String>("handle"),
      NanNewBufferHandle((const char *)&handle, sizeof(OCDoHandle)));

  NanReturnValue(returnValue);
}
