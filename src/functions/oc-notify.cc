#include <v8.h>
#include <node_buffer.h>
#include <nan.h>

#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCNotifyAllObservers) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsNumber);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  NanReturnValue(NanNew<Number>(OCNotifyAllObservers(
      handle, (OCQualityOfService)args[1]->Uint32Value())));
}

NAN_METHOD(bind_OCNotifyListOfObservers) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 5);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsNumber);
  VALIDATE_ARGUMENT_TYPE(args, 3, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 4, IsNumber);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(&handle, args[0]->ToObject())) {
    NanReturnUndefined();
  }

  // Construct the C array of observation IDs.
  Local<Array> obsIds = Local<Array>::Cast(args[1]);
  uint8_t arrayLength = (uint8_t)obsIds->Length();
  uint8_t numberOfIds = (uint8_t)args[2]->Uint32Value();

  // If the array contains fewer IDs than the reported number, we use the lower
  // number
  numberOfIds = (numberOfIds < arrayLength ? numberOfIds : arrayLength);

  OCObservationId *c_observations =
      (OCObservationId *)malloc(numberOfIds * sizeof(OCObservationId));
  if (!c_observations && numberOfIds > 0) {
    NanThrowError(
        "OCNotifyListOfObservers: Failed to allocate list of observers");
    NanReturnUndefined();
  }

  // Populate a C-like array from the V8 array
  int index;
  for (index = 0; index < numberOfIds; index++) {
    Local<Value> oneObservationId = obsIds->Get(index);
    if (!(oneObservationId->IsNumber())) {
      NanThrowTypeError("OCObservationID must satisfy IsNumber()");
      NanReturnUndefined();
    }
    c_observations[index] = (OCObservationId)oneObservationId->Uint32Value();
  }

  Local<Number> returnValue = NanNew<Number>(
      OCNotifyListOfObservers(handle, c_observations, numberOfIds,
                              (const char *)*String::Utf8Value(args[3]),
                              (OCQualityOfService)args[4]->Uint32Value()));

  free(c_observations);

  NanReturnValue(returnValue);
}
