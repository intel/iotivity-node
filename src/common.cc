#include <nan.h>
#include "common.h"

using namespace v8;

Persistent<Function> *persistentJSCallback_new(Local<Function> callback) {
  Persistent<Function> *returnValue = new Persistent<Function>;
  NanAssignPersistent(*returnValue, callback);
  return returnValue;
}

void persistentJSCallback_free(Persistent<Function> *callback) {
  delete callback;
}
