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
}

NODE_MODULE(iotivity, Init)
