#include <v8.h>
#include <node_buffer.h>
#include <nan.h>
#include <map>

#include "../common.h"
#include "../structures/handles.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;
using namespace node;

// Associate the callback info with a resource handle
static std::map<OCResourceHandle, Persistent<Function> *> annotation;

static OCEntityHandlerResult defaultEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, void *context) {
  // Construct arguments to the JS callback and then call it, recording its
  // return value
  Local<Value> jsCallbackArguments[2] = {NanNew<Number>(flag),
                                         js_OCEntityHandlerRequest(request)};
  Local<Value> returnValue = NanMakeCallback(
      NanGetCurrentContext()->Global(),
      NanNew(*(Persistent<Function> *)context), 2, jsCallbackArguments);

  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsNumber, "OCEntityHandler");

  return (OCEntityHandlerResult)(returnValue->Uint32Value());
}

NAN_METHOD(bind_OCCreateResource) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 6);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 3, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 4, IsFunction);
  VALIDATE_ARGUMENT_TYPE(args, 5, IsUint32);

  OCResourceHandle handle = 0;
  Persistent<Function> *callback =
      persistentJSCallback_new(Local<Function>::Cast(args[4]));

  Local<Number> returnValue = NanNew<Number>(OCCreateResource(
      &handle, (const char *)*String::Utf8Value(args[1]),
      (const char *)*String::Utf8Value(args[2]),
      (const char *)*String::Utf8Value(args[3]), defaultEntityHandler,
      (void *)callback, (uint8_t)args[5]->Uint32Value()));

  // Save info to the handle
  annotation[handle] = callback;
  args[0]->ToObject()->Set(NanNew<String>("handle"),
                           js_OCResourceHandle(handle));
  NanReturnValue(returnValue);
}

NAN_METHOD(bind_OCDeleteResource) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);

  OCStackResult returnValue;
  OCResourceHandle handle = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  // Delete the resource identified by the handle
  returnValue = OCDeleteResource(handle);

  if (returnValue == OC_STACK_OK) {
    // If deleting the resource worked, get rid of the entity handler
    Persistent<Function> *callback = annotation[handle];
    annotation.erase(handle);
    if (callback) {
      persistentJSCallback_free(callback);
    }
  }

  NanReturnValue(NanNew<Number>(returnValue));
}

NAN_METHOD(bind_OCBindResourceHandler) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsFunction);

  OCResourceHandle handle = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  Persistent<Function> *callback =
      persistentJSCallback_new(Local<Function>::Cast(args[1]));

  // Replace the existing entity handler with the new callback
  OCStackResult returnValue =
      OCBindResourceHandler(handle, defaultEntityHandler, (void *)callback);

  if (returnValue == OC_STACK_OK) {
    // If setting the new entity handler worked, get rid of the original entity
    // handler and associate the new one with the handle.
    Persistent<Function> *oldCallback = annotation[handle];
    if (oldCallback) {
      persistentJSCallback_free(oldCallback);
    }
    annotation[handle] = callback;
  } else {
    // If the stack was not able to make use of the new entity handler, get rid
    // of the reference we created above.
    persistentJSCallback_free(callback);
  }

  NanReturnValue(NanNew<Number>(returnValue));
}

NAN_METHOD(bind_OCBindResource) {
  NanScope();

  OCResourceHandle collectionHandle = 0, resourceHandle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &collectionHandle)) {
    NanReturnUndefined();
  }
  if (!c_OCResourceHandle(Local<Array>::Cast(args[1]), &resourceHandle)) {
    NanReturnUndefined();
  }

  NanReturnValue(
      NanNew<Number>(OCBindResource(collectionHandle, resourceHandle)));
}

NAN_METHOD(bind_OCBindResourceInterfaceToResource) {
  NanScope();

  OCResourceHandle handle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsString);

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCBindResourceInterfaceToResource(
      handle, (const char *)*String::Utf8Value(args[1]))));
}

NAN_METHOD(bind_OCBindResourceTypeToResource) {
  NanScope();

  OCResourceHandle handle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsString);

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCBindResourceTypeToResource(
      handle, (const char *)*String::Utf8Value(args[1]))));
}
