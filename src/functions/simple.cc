#include <nan.h>
#include "../common.h"
#include "../structures/oc-platform-info.h"
#include "../structures/string-primitive.h"

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
      (const char *)(args[0]->IsString() ? (*String::Utf8Value(args[0])) : 0),
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

static bool c_OCDeviceInfo(Local<Object> devInfo, OCDeviceInfo *info) {
  Local<Value> deviceName = devInfo->Get(NanNew<String>("deviceName"));
  VALIDATE_VALUE_TYPE(deviceName, IsString, "deviceInfo.deviceName", false);
  char *devName;
  if (!c_StringNew(deviceName->ToString(), &devName)) {
    return false;
  }
  info->deviceName = devName;
  return true;
}

NAN_METHOD(bind_OCSetDeviceInfo) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCDeviceInfo info;

  if (!c_OCDeviceInfo(args[0]->ToObject(), &info)) {
    NanReturnUndefined();
  }

  OCStackResult result = OCSetDeviceInfo(info);

  free(info.deviceName);

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCSetPlatformInfo) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCPlatformInfo info;

  if (!c_OCPlatformInfo(args[0]->ToObject(), &info)) {
    NanReturnUndefined();
  }

  OCStackResult result = OCSetPlatformInfo(info);

  c_OCPlatformInfoFreeMembers(&info);

  NanReturnValue(NanNew<Number>(result));
}
