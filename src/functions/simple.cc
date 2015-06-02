#include <nan.h>
#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

NAN_METHOD(bind_OCInit) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 3);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 0, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsNumber);

  NanReturnValue(NanNew<Number>(OCInit(
      (const char*)(args[0]->IsString() ? (*String::Utf8Value(args[0])) : 0),
      (uint16_t)args[1]->ToUint32()->Value(),
      (OCMode)args[2]->ToNumber()->Value())));
}

NAN_METHOD(bind_OCStop) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCStop()));
}

NAN_METHOD(bind_OCProcess) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCProcess()));
}

NAN_METHOD(bind_OCStartPresence) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsUint32);

  NanReturnValue(
      NanNew<Number>(OCStartPresence((uint32_t)args[0]->Uint32Value())));
}

NAN_METHOD(bind_OCStopPresence) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCStopPresence()));
}

NAN_METHOD(bind_OCDevAddrToIPv4Addr) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);

  OCDevAddr c_addr;

  if (!c_OCDevAddr(args[0]->ToObject(), &c_addr)) {
    NanReturnUndefined();
  }

  uint8_t a = 0, b = 0, c = 0, d = 0;

  OCStackResult result =
      (OCStackResult)OCDevAddrToIPv4Addr(&c_addr, &a, &b, &c, &d);

  Local<Array> array = Local<Array>::Cast(args[1]);

  array->Set(0, NanNew<Number>(a));
  array->Set(1, NanNew<Number>(b));
  array->Set(2, NanNew<Number>(c));
  array->Set(3, NanNew<Number>(d));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCDevAddrToPort) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsObject);

  OCDevAddr c_addr;

  if (!c_OCDevAddr(args[0]->ToObject(), &c_addr)) {
    NanReturnUndefined();
  }

  uint16_t port = 0;

  OCStackResult result = (OCStackResult)OCDevAddrToPort(&c_addr, &port);

  args[1]->ToObject()->Set(NanNew<String>("port"), NanNew<Number>(port));

  NanReturnValue(NanNew<Number>(result));
}
