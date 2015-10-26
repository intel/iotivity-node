#include "../common.h"
#include "../structures.h"
#include "oc-set-default-device-entity-handler.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

Nan::Callback *g_currentCallback = 0;

static OCEntityHandlerResult defaultDeviceEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, char *uri,
    void *context) {
  Local<Value> jsCallbackArguments[3] = {
      Nan::New(flag), js_OCEntityHandlerRequest(request),
      (uri ? Nan::New<Value>((Handle<String>)Nan::New(uri).ToLocalChecked())
           : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()))};

  Local<Value> returnValue =
      ((Nan::Callback *)context)->Call(3, jsCallbackArguments);

  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCDeviceEntityHandler");

  return (OCEntityHandlerResult)(returnValue->Uint32Value());
}

NAN_METHOD(bind_OCSetDefaultDeviceEntityHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 0, IsFunction);

  OCDeviceEntityHandler newHandler = 0;
  Nan::Callback *newCallback = 0, *callbackToDelete = 0;

  if (info[0]->IsFunction()) {
    newCallback = new Nan::Callback(Local<Function>::Cast(info[0]));
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

  info.GetReturnValue().Set(Nan::New(result));
}
