#include <node.h>
#include <nan.h>

#include "enums.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

#define SET_ENUM(destination, enumName) \
  (destination)->Set(NanNew<String>(#enumName), bind_##enumName())

// The rest of this file is generated

static Local<Object> bind_OCVirtualResources() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_WELL_KNOWN_URI);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_DEVICE_URI);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_PLATFORM_URI);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_RESOURCE_TYPES_URI);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_PRESENCE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_MAX_VIRTUAL_RESOURCES);

  return returnValue;
}

static Local<Object> bind_OCMethod() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_NOMETHOD);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_GET);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_PUT);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_POST);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_DELETE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_OBSERVE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_OBSERVE_ALL);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_CANCEL_OBSERVE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_REST_PRESENCE);

  return returnValue;
}

static Local<Object> bind_OCMode() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_CLIENT);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_SERVER);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_CLIENT_SERVER);

  return returnValue;
}

static Local<Object> bind_OCQualityOfService() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_LOW_QOS);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_MEDIUM_QOS);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_HIGH_QOS);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_NA_QOS);

  return returnValue;
}

static Local<Object> bind_OCResourceProperty() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_ACTIVE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_DISCOVERABLE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_OBSERVABLE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_SLOW);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_SECURE);

  return returnValue;
}

static Local<Object> bind_OCTransportProtocolID() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_INVALID_ID);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_COAP_ID);

  return returnValue;
}

static Local<Object> bind_OCConnectivityType() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_IPV4);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_IPV6);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EDR);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_LE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_ALL);

  return returnValue;
}

static Local<Object> bind_OCStackResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_OK);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_RESOURCE_CREATED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_RESOURCE_DELETED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_CONTINUE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_URI);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_QUERY);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_IP);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_PORT);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_CALLBACK);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_METHOD);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_PARAM);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_OBSERVE_PARAM);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_NO_MEMORY);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_COMM_ERROR);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_TIMEOUT);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_ADAPTER_NOT_ENABLED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_NOTIMPL);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_NO_RESOURCE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_RESOURCE_ERROR);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_SLOW_RESOURCE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_DUPLICATE_REQUEST);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_NO_OBSERVERS);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_OBSERVER_NOT_FOUND);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_VIRTUAL_DO_NOT_HANDLE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_OPTION);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_MALFORMED_RESPONSE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_PERSISTENT_BUFFER_REQUIRED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_REQUEST_HANDLE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_DEVICE_INFO);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_INVALID_JSON);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_PRESENCE_STOPPED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_PRESENCE_TIMEOUT);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_PRESENCE_DO_NOT_HANDLE);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_ERROR);

  return returnValue;
}

static Local<Object> bind_OCObserveAction() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_OBSERVE_REGISTER);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_OBSERVE_DEREGISTER);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_OBSERVE_NO_OPTION);

  return returnValue;
}

static Local<Object> bind_OCEntityHandlerResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_OK);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_ERROR);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_RESOURCE_CREATED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_RESOURCE_DELETED);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_SLOW);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_FORBIDDEN);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_EH_RESOURCE_NOT_FOUND);

  return returnValue;
}

static Local<Object> bind_OCEntityHandlerFlag() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_REQUEST_FLAG);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_OBSERVE_FLAG);

  return returnValue;
}

static Local<Object> bind_OCStackApplicationResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_DELETE_TRANSACTION);
  SET_CONSTANT_MEMBER(returnValue, Number, OC_STACK_KEEP_TRANSACTION);

  return returnValue;
}

void InitEnums(Handle<Object> exports) {
  SET_ENUM(exports, OCVirtualResources);
  SET_ENUM(exports, OCMethod);
  SET_ENUM(exports, OCMode);
  SET_ENUM(exports, OCQualityOfService);
  SET_ENUM(exports, OCResourceProperty);
  SET_ENUM(exports, OCTransportProtocolID);
  SET_ENUM(exports, OCConnectivityType);
  SET_ENUM(exports, OCStackResult);
  SET_ENUM(exports, OCObserveAction);
  SET_ENUM(exports, OCEntityHandlerResult);
  SET_ENUM(exports, OCEntityHandlerFlag);
  SET_ENUM(exports, OCStackApplicationResult);
}
