#include <v8.h>
#include <nan.h>

#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocstack.h>
#include <ocpayload.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCNotifyAllObservers) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsNumber);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCNotifyAllObservers(
      handle, (OCQualityOfService)args[1]->Uint32Value())));
}

NAN_METHOD(bind_OCNotifyListOfObservers) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 4);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 3, IsNumber);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  // Construct the C array of observation IDs.
  Local<Array> obsIds = Local<Array>::Cast(args[1]);
  uint8_t arrayLength = (uint8_t)obsIds->Length();

  OCObservationId *c_observations = 0;

  if (arrayLength > 0) {
    c_observations =
        (OCObservationId *)malloc(arrayLength * sizeof(OCObservationId));
    if (!c_observations && arrayLength > 0) {
      NanThrowError(
          "OCNotifyListOfObservers: Failed to allocate list of observers");
      NanReturnUndefined();
    }
  }

  // Populate a C-like array from the V8 array
  int index;
  for (index = 0; index < arrayLength; index++) {
    Local<Value> oneObservationId = obsIds->Get(index);
    if (!(oneObservationId->IsUint32())) {
      free(c_observations);
      NanThrowTypeError("OCObservationID must satisfy IsNumber()");
      NanReturnUndefined();
    }
    c_observations[index] = (OCObservationId)oneObservationId->Uint32Value();
  }

  OCRepPayload *payload = 0;
  if (!c_OCPayload(args[2]->ToObject(), (OCPayload **)&payload)) {
    free(c_observations);
    NanReturnUndefined();
  }

  Local<Number> returnValue = NanNew<Number>(
      OCNotifyListOfObservers(handle, c_observations, arrayLength, payload,
                              (OCQualityOfService)args[3]->Uint32Value()));

  free(c_observations);
  OCRepPayloadDestroy(payload);

  NanReturnValue(returnValue);
}
