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

  SET_CONSTANT_MEMBER(exports, Number, DEV_ADDR_SIZE_MAX);
  SET_CONSTANT_MEMBER(exports, Number, MAX_RESPONSE_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_REQUEST_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_URI_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_QUERY_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_MANUFACTURER_NAME_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_MANUFACTURER_URL_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_CONTAINED_RESOURCES);
  SET_CONSTANT_MEMBER(exports, Number, MAX_HEADER_OPTIONS);
  SET_CONSTANT_MEMBER(exports, Number, MAX_HEADER_OPTION_DATA_LENGTH);
  SET_CONSTANT_MEMBER(exports, Number, MAX_CB_TIMEOUT_SECONDS);

  SET_CONSTANT_MEMBER(exports, String, OC_MULTICAST_DISCOVERY_URI);
  SET_CONSTANT_MEMBER(exports, String, OC_WELL_KNOWN_QUERY);
}

NODE_MODULE(iotivity, Init)
