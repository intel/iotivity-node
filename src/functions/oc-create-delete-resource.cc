#include <v8.h>
#include <node_buffer.h>
#include <nan.h>
#include <map>

#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

// Associate the callback info with a resource handle
static std::map<OCResourceHandle, callback_info *> annotation;

// Marshaller for OCEntityHandler callback
// defaultEntityHandler is placed in a closure each time someone calls
// OCCreateResource. Closures differ from one another only by the value of
// jsCallbackInPersistent, which is a persistent reference to the JS callback.
// When the C API executes one of the closures, we construct a call to the JS
// callback we find at jsCallbackInPersistent, and pass the return value from
// the callback back to the C API.
static void defaultEntityHandler(ffi_cif *cif,
                                 OCEntityHandlerResult *returnValueLocation,
                                 void **arguments,
                                 void *jsCallbackInPersistent) {
  // Construct arguments to the JS callback and then call it, recording its
  // return value
  Local<Value> jsCallbackArguments[2] = {
      NanNew<Number>((double)*(OCEntityHandlerFlag *)(arguments[0])),
      js_OCEntityHandlerRequest(*(OCEntityHandlerRequest **)(arguments[1]))};
  Local<Value> returnValue =
      NanMakeCallback(NanGetCurrentContext()->Global(),
                      NanNew(*(Persistent<Function> *)jsCallbackInPersistent),
                      2, jsCallbackArguments);

  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsNumber, "OCEntityHandler");

  *returnValueLocation =
      (OCEntityHandlerResult)(returnValue->ToNumber()->Value());
}

// Create a callback_info structure for a given JS callback
#define newInfoForJSCallback(callback)                                     \
  callback_info_new((void *)persistentJSCallback_new((callback)),          \
                    (UserDataRemover)persistentJSCallback_free,            \
                    &ffi_type_uint32, (Marshaller)defaultEntityHandler, 2, \
                    &ffi_type_uint32, &ffi_type_pointer)

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
  callback_info *info = 0;

  // Create a new callback
  info = newInfoForJSCallback(Local<Function>::Cast(args[4]));
  if (!info) {
    NanThrowError("OCCreateResource: Unable to allocate C callback");
    NanReturnUndefined();
  }

  Local<Number> returnValue = NanNew<Number>(
      OCCreateResource(&handle, (const char *)*String::Utf8Value(args[1]),
                       (const char *)*String::Utf8Value(args[2]),
                       (const char *)*String::Utf8Value(args[3]),
                       (OCEntityHandler)(info->resultingFunction),
                       (uint8_t)args[5]->Uint32Value()));

  // Save info to the handle
  annotation[handle] = info;
  js_OCResourceHandle(args[0]->ToObject(), handle);

  NanReturnValue(returnValue);
}

NAN_METHOD(bind_OCDeleteResource) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCStackResult returnValue;
  OCResourceHandle handle = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  // Delete the resource identified by the handle
  returnValue = OCDeleteResource(handle);

  if (returnValue == OC_STACK_OK) {
    // If deleting the resource worked, get rid of the entity handler
    callback_info *info = annotation[handle];
    annotation.erase(handle);
    if (info) {
      callback_info_free(info);
    }
  }

  NanReturnValue(NanNew<Number>(returnValue));
}

NAN_METHOD(bind_OCBindResourceHandler) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsFunction);

  OCResourceHandle handle = 0;
  callback_info *info = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  // Create a new callback
  info = newInfoForJSCallback(Local<Function>::Cast(args[1]));
  if (!info) {
    NanThrowError("OCBindResourceHandler: Unable to allocate C callback");
    NanReturnUndefined();
  }

  // Replace the existing entity handler with the new callback
  OCStackResult returnValue =
      OCBindResourceHandler(handle, (OCEntityHandler)(info->resultingFunction));

  if (returnValue == OC_STACK_OK) {
    // If setting the new entity handler worked, get rid of the original entity
    // handler and associate the new one with the handle.
    callback_info *old_info = annotation[handle];
    if (old_info) {
      callback_info_free(old_info);
    }
    annotation[handle] = info;
  } else {
    // If the stack was not able to make use of the new entity handler, get rid
    // of the callback we created above.
    callback_info_free(info);
  }

  NanReturnValue(NanNew<Number>(returnValue));
}

NAN_METHOD(bind_OCBindResource) {
  NanScope();

  OCResourceHandle collectionHandle = 0, resourceHandle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsObject);

  if (!c_OCResourceHandle(&collectionHandle, args[0]->ToObject())) {
    NanReturnUndefined();
  }
  if (!c_OCResourceHandle(&resourceHandle, args[1]->ToObject())) {
    NanReturnUndefined();
  }

  NanReturnValue(
      NanNew<Number>(OCBindResource(collectionHandle, resourceHandle)));
}

NAN_METHOD(bind_OCBindResourceInterfaceToResource) {
  NanScope();

  OCResourceHandle handle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsString);

  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCBindResourceInterfaceToResource(
      handle, (const char *)*String::Utf8Value(args[1]))));
}

NAN_METHOD(bind_OCBindResourceTypeToResource) {
  NanScope();

  OCResourceHandle handle = 0;

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsString);

  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCBindResourceTypeToResource(
      handle, (const char *)*String::Utf8Value(args[1]))));
}
