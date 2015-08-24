#include <node.h>
#include <nan.h>

extern "C" {
#include <ocstack.h>
}

#include "functions.h"
#include "enums.h"
#include "constants.h"

using namespace v8;

void Init(Handle<Object> exports, Handle<Object> module) {
  InitFunctions(exports, module);
  InitEnums(exports);
  InitConstants(exports);

  // FIXME: Remove the definition of the sid length once the stack starts
  // providing it
  exports->Set(NanNew<String>("UUID_SIZE"), NanNew<Number>(16));
}

NODE_MODULE(iotivity, Init)
