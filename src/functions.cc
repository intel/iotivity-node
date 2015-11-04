#include <node.h>
#include <nan.h>

#include "functions/oc-cancel.h"
#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/oc-do-response.h"
#include "functions/oc-notify.h"
#include "functions/oc-random.h"
#include "functions/oc-set-default-device-entity-handler.h"
#include "functions/simple.h"

using namespace v8;

#define SET_FUNCTION(destination, functionName)                              \
  Nan::Set((destination), Nan::New(#functionName).ToLocalChecked(),          \
           Nan::GetFunction(Nan::New<FunctionTemplate>(bind_##functionName)) \
               .ToLocalChecked())

void InitFunctions(Handle<Object> exports, Handle<Object> module) {
  SET_FUNCTION(exports, OCBindResource);
  SET_FUNCTION(exports, OCBindResourceHandler);
  SET_FUNCTION(exports, OCBindResourceInterfaceToResource);
  SET_FUNCTION(exports, OCBindResourceTypeToResource);
  SET_FUNCTION(exports, OCCancel);
  SET_FUNCTION(exports, OCConvertUuidToString);
  SET_FUNCTION(exports, OCCreateResource);
  SET_FUNCTION(exports, OCDeleteResource);
  SET_FUNCTION(exports, OCDoResource);
  SET_FUNCTION(exports, OCDoResponse);
  SET_FUNCTION(exports, OCFillRandomMem);
  SET_FUNCTION(exports, OCGenerateUuid);
  SET_FUNCTION(exports, OCGenerateUuidString);
  SET_FUNCTION(exports, OCGetNumberOfResourceInterfaces);
  SET_FUNCTION(exports, OCGetNumberOfResources);
  SET_FUNCTION(exports, OCGetNumberOfResourceTypes);
  SET_FUNCTION(exports, OCGetRandom);
  SET_FUNCTION(exports, OCGetRandomByte);
  SET_FUNCTION(exports, OCGetRandomRange);
  SET_FUNCTION(exports, OCGetResourceHandle);
  SET_FUNCTION(exports, OCGetResourceHandleFromCollection);
  SET_FUNCTION(exports, OCGetResourceHandler);
  SET_FUNCTION(exports, OCGetResourceInterfaceName);
  SET_FUNCTION(exports, OCGetResourceProperties);
  SET_FUNCTION(exports, OCGetResourceTypeName);
  SET_FUNCTION(exports, OCGetResourceUri);
  SET_FUNCTION(exports, OCInit);
  SET_FUNCTION(exports, OCNotifyAllObservers);
  SET_FUNCTION(exports, OCNotifyListOfObservers);
  SET_FUNCTION(exports, OCProcess);
  SET_FUNCTION(exports, OCSeedRandom);
  SET_FUNCTION(exports, OCSetDefaultDeviceEntityHandler);
  SET_FUNCTION(exports, OCSetDeviceInfo);
  SET_FUNCTION(exports, OCSetPlatformInfo);
  SET_FUNCTION(exports, OCStartPresence);
  SET_FUNCTION(exports, OCStop);
  SET_FUNCTION(exports, OCStopPresence);
  SET_FUNCTION(exports, OCUnBindResource);
#ifdef TESTING
  SET_FUNCTION(exports, __compareResourceHandles);
  SET_FUNCTION(exports, __compareDoHandles);
#endif /* def TESTING */
}
