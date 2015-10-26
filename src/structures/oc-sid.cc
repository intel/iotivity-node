#include <nan.h>
#include "oc-sid.h"
#include "../common.h"

extern "C" {
#include <string.h>
#include <ocrandom.h>
}

using namespace v8;

void fillJSSid(Local<Array> jsSid, uint8_t *sid) {
  int index;
  for (index = 0; index < UUID_SIZE; index++) {
    jsSid->Set(index, Nan::New(sid[index]));
  }
}

Local<Array> js_SID(uint8_t *sid) {
  Local<Array> returnValue = Nan::New<Array>(UUID_SIZE);

  fillJSSid(returnValue, sid);

  return returnValue;
}

bool c_SID(Local<Array> jsSid, uint8_t *sid) {
  int index;
  uint8_t result[UUID_SIZE];

  if (jsSid->Length() > UUID_SIZE) {
    Nan::ThrowRangeError("SID length must be UUID_SIZE");
    return false;
  }

  for (index = 0; index < UUID_SIZE; index++) {
    Local<Value> oneByte = Nan::Get(jsSid, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE(oneByte, IsUint32, "SID byte", false);
    result[index] = oneByte->Uint32Value();
  }

  memcpy(sid, result, UUID_SIZE * sizeof(uint8_t));
  return true;
}
