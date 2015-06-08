#include <node.h>
#include <nan.h>

#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/oc-do-response.h"
#include "functions/oc-notify.h"
#include "functions/simple.h"

using namespace v8;

#define SET_FUNCTION(destination, functionName) \
  (destination)                                 \
      ->Set(NanNew<String>(#functionName),      \
            NanNew<FunctionTemplate>(bind_##functionName)->GetFunction())

void InitFunctions(Handle<Object> exports, Handle<Object> module) {
  SET_FUNCTION(exports, OCInit);
  SET_FUNCTION(exports, OCStop);
  SET_FUNCTION(exports, OCProcess);
  SET_FUNCTION(exports, OCDevAddrToIPv4Addr);
  SET_FUNCTION(exports, OCDevAddrToPort);
  SET_FUNCTION(exports, OCCreateResource);
  SET_FUNCTION(exports, OCBindResourceHandler);
  SET_FUNCTION(exports, OCBindResource);
  SET_FUNCTION(exports, OCBindResourceInterfaceToResource);
  SET_FUNCTION(exports, OCBindResourceTypeToResource);
  SET_FUNCTION(exports, OCDeleteResource);
  SET_FUNCTION(exports, OCStartPresence);
  SET_FUNCTION(exports, OCStopPresence);
  SET_FUNCTION(exports, OCDoResource);
  SET_FUNCTION(exports, OCDoResponse);
  SET_FUNCTION(exports, OCNotifyAllObservers);
  SET_FUNCTION(exports, OCNotifyListOfObservers);
}
