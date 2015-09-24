#include <nan.h>
#include "../common.h"
#include "../structures/oc-sid.h"

extern "C" {
#include <ocrandom.h>
}

using namespace v8;

NAN_METHOD(bind_OCConvertUuidToString) {
  NanScope();

  OCRandomUuidResult result;
  uint8_t sid[UUID_SIZE] = {0};
  char stringSid[UUID_STRING_SIZE] = "";

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsObject);

  if (!c_SID(Local<Array>::Cast(args[0]), sid)) {
    NanReturnUndefined();
  }

  result = OCConvertUuidToString(sid, stringSid);

  Local<Object> receptacle = Local<Object>::Cast(args[1]);

  receptacle->Set(NanNew<String>("sid"), NanNew<String>(stringSid));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCFillRandomMem) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);

  Local<Array> destination = Local<Array>::Cast(args[0]);
  ;
  uint16_t length = destination->Length();
  uint8_t *cDestination = (uint8_t *)malloc(length * sizeof(uint8_t));
  memset(cDestination, 0, length * sizeof(uint8_t));
  OCFillRandomMem(cDestination, length);
  for (int index = 0; index < length; index++) {
    destination->Set(index, NanNew<Number>(cDestination[index]));
  }
  free(cDestination);

  NanReturnUndefined();
}

NAN_METHOD(bind_OCGenerateUuid) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);

  Local<Array> destination = Local<Array>::Cast(args[0]);
  uint32_t length = destination->Length();
  if (length != UUID_SIZE) {
    NanThrowRangeError("Array is not of length UUID_SIZE");
    NanReturnUndefined();
  }
  uint8_t cDestination[UUID_SIZE] = {0};
  OCRandomUuidResult result = OCGenerateUuid(cDestination);
  fillJSSid(destination, cDestination);
  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGenerateUuidString) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  char resultingString[UUID_STRING_SIZE] = "";
  OCRandomUuidResult result = OCGenerateUuidString(resultingString);
  Local<Object> destination = Local<Object>::Cast(args[0]);
  destination->Set(NanNew<String>("sid"), NanNew<String>(resultingString));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGetRandom) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 0);

  NanReturnValue(NanNew<Number>(OCGetRandom()));
}

NAN_METHOD(bind_OCGetRandomByte) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 0);

  NanReturnValue(NanNew<Number>(OCGetRandomByte()));
}

NAN_METHOD(bind_OCGetRandomRange) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);

  NanReturnValue(NanNew<Number>(
      OCGetRandomRange(args[0]->Uint32Value(), args[1]->Uint32Value())));
}

NAN_METHOD(bind_OCSeedRandom) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 0);

  NanReturnValue(NanNew<Number>(OCSeedRandom()));
}
