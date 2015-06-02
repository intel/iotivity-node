#include <node.h>
#include <nan.h>

extern "C" {
#include <ocstack.h>
}

#include "functions.h"
#include "enums.h"

using namespace v8;

void Init(Handle<Object> exports, Handle<Object> module) {
  InitFunctions(exports, module);
  InitEnums(exports);

  exports->Set(NanNew<String>("MAX_RESPONSE_LENGTH"),
               NanNew<Number>(MAX_RESPONSE_LENGTH));

  exports->Set(NanNew<String>("MAX_URI_LENGTH"),
               NanNew<Number>(MAX_URI_LENGTH));

  exports->Set(NanNew<String>("MAX_HEADER_OPTIONS"),
               NanNew<Number>(MAX_HEADER_OPTIONS));

  exports->Set(NanNew<String>("MAX_HEADER_OPTION_DATA_LENGTH"),
               NanNew<Number>(MAX_HEADER_OPTION_DATA_LENGTH));
}

NODE_MODULE(iotivity, Init)
