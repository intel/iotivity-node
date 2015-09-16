#include "../common.h"
#include "../structures.h"
#include "oc-set-default-device-entity-handler.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

NanCallback *g_currentCallback = 0;

static OCEntityHandlerResult defaultDeviceEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, char *uri,
    void *context) {
  Local<Value> jsCallbackArguments[3] = {
      NanNew<Number>(flag), js_OCEntityHandlerRequest(request),
      (uri ? NanNew<Value>(NanNew<String>(uri))
           : NanNew<Value>(NanUndefined()))};

  Local<Value> returnValue = ((NanCallback *)context)->Call(
      NanGetCurrentContext()->Global(), 3, jsCallbackArguments);

  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCDeviceEntityHandler");

  return (OCEntityHandlerResult)(returnValue->Uint32Value());
}

NAN_METHOD(bind_OCSetDefaultDeviceEntityHandler) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 0, IsFunction);

  OCDeviceEntityHandler newHandler = 0;
  NanCallback *newCallback = 0, *callbackToDelete = 0;

  if (args[0]->IsFunction()) {
    newCallback = new NanCallback(Local<Function>::Cast(args[0]));
    newHandler = defaultDeviceEntityHandler;
  }

  OCStackResult result =
      OCSetDefaultDeviceEntityHandler(newHandler, (void *)newCallback);

  if (result == OC_STACK_OK) {
    callbackToDelete = g_currentCallback;
    g_currentCallback = newCallback;
  } else {
    callbackToDelete = newCallback;
  }

  if (callbackToDelete) {
    delete callbackToDelete;
  }

  NanReturnValue(NanNew<Number>(result));
}
