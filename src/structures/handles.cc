#include <nan.h>
#include "handles.h"
#include "../common.h"

using namespace v8;

static Local<Array> jsArrayFromBytes(unsigned char *bytes, size_t length) {
  size_t index;
  Local<Array> returnValue = NanNew<Array>(length);

  for (index = 0; index < length; index++) {
    returnValue->Set(index, NanNew<Number>(bytes[index]));
  }
  return returnValue;
}

static bool fillCArrayFromJSArray(unsigned char *bytes, size_t length,
                                  Local<Array> array) {
  size_t index, arrayLength;

  arrayLength = array->Length();
  if (arrayLength != length) {
    NanThrowError("byte array has the wrong length");
    return false;
  }

  for (index = 0; index < length; index++) {
    Local<Value> byte = array->Get(index);
    VALIDATE_VALUE_TYPE(byte, IsUint32, "byte array item", false);
    bytes[index] = (unsigned char)(byte->Uint32Value());
  }

  return true;
}

Local<Array> js_OCResourceHandle(OCResourceHandle handle) {
  return jsArrayFromBytes(((unsigned char *)(&handle)),
                          sizeof(OCResourceHandle));
}

#define C_HANDLE(jsHandle, cType, destination)                         \
  cType local;                                                         \
  if (!fillCArrayFromJSArray(((unsigned char *)&local), sizeof(cType), \
                             jsHandle)) {                              \
    return false;                                                      \
  }                                                                    \
  *destination = local;                                                \
  return true;

bool c_OCResourceHandle(Local<Array> handle, OCResourceHandle *p_cHandle) {
  C_HANDLE(handle, OCResourceHandle, p_cHandle);
}

bool c_OCRequestHandle(Local<Array> handle, OCRequestHandle *p_cHandle) {
  C_HANDLE(handle, OCRequestHandle, p_cHandle);
}

bool c_OCDoHandle(Local<Array> handle, OCDoHandle *p_cHandle) {
  C_HANDLE(handle, OCDoHandle, p_cHandle);
}
Local<Array> js_OCRequestHandle(OCRequestHandle handle) {
  return jsArrayFromBytes(((unsigned char *)(&handle)),
                          sizeof(OCRequestHandle));
}

Local<Array> js_OCDoHandle(OCDoHandle handle) {
  return jsArrayFromBytes(((unsigned char *)(&handle)), sizeof(OCDoHandle));
}
