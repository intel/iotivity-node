#include <node.h>
#include <nan.h>

extern "C" {
#include <ocstack.h>
}

using namespace v8;

#define SET_ENUM_MEMBER(destination, name)                      \
  (destination)                                                 \
      ->ForceSet(NanNew<String>(#name), NanNew<Number>((name)), \
                 static_cast<PropertyAttribute>(ReadOnly || DontDelete));

static Local<Object> bind_OCEntityHandlerFlag() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_REQUEST_FLAG);
  SET_ENUM_MEMBER(returnValue, OC_OBSERVE_FLAG);

  return returnValue;
}

static Local<Object> bind_OCEntityHandlerResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_EH_OK);
  SET_ENUM_MEMBER(returnValue, OC_EH_ERROR);
  SET_ENUM_MEMBER(returnValue, OC_EH_RESOURCE_CREATED);
  SET_ENUM_MEMBER(returnValue, OC_EH_RESOURCE_DELETED);
  SET_ENUM_MEMBER(returnValue, OC_EH_SLOW);
  SET_ENUM_MEMBER(returnValue, OC_EH_FORBIDDEN);
  SET_ENUM_MEMBER(returnValue, OC_EH_RESOURCE_NOT_FOUND);

  return returnValue;
}

static Local<Object> bind_OCMethod() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_REST_NOMETHOD);
  SET_ENUM_MEMBER(returnValue, OC_REST_GET);
  SET_ENUM_MEMBER(returnValue, OC_REST_PUT);
  SET_ENUM_MEMBER(returnValue, OC_REST_POST);
  SET_ENUM_MEMBER(returnValue, OC_REST_DELETE);
  SET_ENUM_MEMBER(returnValue, OC_REST_OBSERVE);
  SET_ENUM_MEMBER(returnValue, OC_REST_OBSERVE_ALL);
  SET_ENUM_MEMBER(returnValue, OC_REST_CANCEL_OBSERVE);
  SET_ENUM_MEMBER(returnValue, OC_REST_PRESENCE);

  return returnValue;
}

static Local<Object> bind_OCMode() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_CLIENT);
  SET_ENUM_MEMBER(returnValue, OC_SERVER);
  SET_ENUM_MEMBER(returnValue, OC_CLIENT_SERVER);

  return returnValue;
}

static Local<Object> bind_OCObserveAction() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_OBSERVE_REGISTER);
  SET_ENUM_MEMBER(returnValue, OC_OBSERVE_DEREGISTER);
  SET_ENUM_MEMBER(returnValue, OC_OBSERVE_NO_OPTION);

  return returnValue;
}

static Local<Object> bind_OCQualityOfService() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_LOW_QOS);
  SET_ENUM_MEMBER(returnValue, OC_MEDIUM_QOS);
  SET_ENUM_MEMBER(returnValue, OC_HIGH_QOS);
  SET_ENUM_MEMBER(returnValue, OC_NA_QOS);

  return returnValue;
}

static Local<Object> bind_OCResourceProperty() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_RES_PROP_NONE);
  SET_ENUM_MEMBER(returnValue, OC_ACTIVE);
  SET_ENUM_MEMBER(returnValue, OC_DISCOVERABLE);
  SET_ENUM_MEMBER(returnValue, OC_OBSERVABLE);
  SET_ENUM_MEMBER(returnValue, OC_SLOW);
  SET_ENUM_MEMBER(returnValue, OC_SECURE);

  return returnValue;
}

static Local<Object> bind_OCStackApplicationResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_STACK_DELETE_TRANSACTION);
  SET_ENUM_MEMBER(returnValue, OC_STACK_KEEP_TRANSACTION);

  return returnValue;
}

static Local<Object> bind_OCStackResult() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_STACK_OK);
  SET_ENUM_MEMBER(returnValue, OC_STACK_RESOURCE_CREATED);
  SET_ENUM_MEMBER(returnValue, OC_STACK_RESOURCE_DELETED);
  SET_ENUM_MEMBER(returnValue, OC_STACK_CONTINUE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_URI);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_QUERY);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_IP);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_PORT);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_CALLBACK);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_METHOD);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_PARAM);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_OBSERVE_PARAM);
  SET_ENUM_MEMBER(returnValue, OC_STACK_NO_MEMORY);
  SET_ENUM_MEMBER(returnValue, OC_STACK_COMM_ERROR);
  SET_ENUM_MEMBER(returnValue, OC_STACK_TIMEOUT);
  SET_ENUM_MEMBER(returnValue, OC_STACK_ADAPTER_NOT_ENABLED);
  SET_ENUM_MEMBER(returnValue, OC_STACK_NOTIMPL);
  SET_ENUM_MEMBER(returnValue, OC_STACK_NO_RESOURCE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_RESOURCE_ERROR);
  SET_ENUM_MEMBER(returnValue, OC_STACK_SLOW_RESOURCE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_DUPLICATE_REQUEST);
  SET_ENUM_MEMBER(returnValue, OC_STACK_NO_OBSERVERS);
  SET_ENUM_MEMBER(returnValue, OC_STACK_OBSERVER_NOT_FOUND);
  SET_ENUM_MEMBER(returnValue, OC_STACK_VIRTUAL_DO_NOT_HANDLE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_OPTION);
  SET_ENUM_MEMBER(returnValue, OC_STACK_MALFORMED_RESPONSE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_PERSISTENT_BUFFER_REQUIRED);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_REQUEST_HANDLE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_DEVICE_INFO);
  SET_ENUM_MEMBER(returnValue, OC_STACK_INVALID_JSON);
  SET_ENUM_MEMBER(returnValue, OC_STACK_PRESENCE_STOPPED);
  SET_ENUM_MEMBER(returnValue, OC_STACK_PRESENCE_TIMEOUT);
  SET_ENUM_MEMBER(returnValue, OC_STACK_PRESENCE_DO_NOT_HANDLE);
  SET_ENUM_MEMBER(returnValue, OC_STACK_ERROR);

  return returnValue;
}

static Local<Object> bind_OCTransportProtocolID() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_INVALID_ID);
  SET_ENUM_MEMBER(returnValue, OC_COAP_ID);

  return returnValue;
}

static Local<Object> bind_OCVirtualResources() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_WELL_KNOWN_URI);
  SET_ENUM_MEMBER(returnValue, OC_DEVICE_URI);
  SET_ENUM_MEMBER(returnValue, OC_PLATFORM_URI);
  SET_ENUM_MEMBER(returnValue, OC_RESOURCE_TYPES_URI);
  SET_ENUM_MEMBER(returnValue, OC_PRESENCE);
  SET_ENUM_MEMBER(returnValue, OC_MAX_VIRTUAL_RESOURCES);

  return returnValue;
}

static Local<Object> bind_OCConnectivityType() {
  Local<Object> returnValue = NanNew<Object>();

  SET_ENUM_MEMBER(returnValue, OC_IPV4);
  SET_ENUM_MEMBER(returnValue, OC_IPV6);
  SET_ENUM_MEMBER(returnValue, OC_EDR);
  SET_ENUM_MEMBER(returnValue, OC_LE);
  SET_ENUM_MEMBER(returnValue, OC_ALL);

  return returnValue;
}

void InitEnums(Handle<Object> exports) {
  exports->Set(NanNew<String>("OCEntityHandlerFlag"),
               bind_OCEntityHandlerFlag());
  exports->Set(NanNew<String>("OCEntityHandlerResult"),
               bind_OCEntityHandlerResult());
  exports->Set(NanNew<String>("OCMethod"), bind_OCMethod());
  exports->Set(NanNew<String>("OCMode"), bind_OCMode());
  exports->Set(NanNew<String>("OCObserveAction"), bind_OCObserveAction());
  exports->Set(NanNew<String>("OCQualityOfService"), bind_OCQualityOfService());
  exports->Set(NanNew<String>("OCResourceProperty"), bind_OCResourceProperty());
  exports->Set(NanNew<String>("OCStackApplicationResult"),
               bind_OCStackApplicationResult());
  exports->Set(NanNew<String>("OCStackResult"), bind_OCStackResult());
  exports->Set(NanNew<String>("OCTransportProtocolID"),
               bind_OCTransportProtocolID());
  exports->Set(NanNew<String>("OCVirtualResources"), bind_OCVirtualResources());
  exports->Set(NanNew<String>("OCConnectivityType"), bind_OCConnectivityType());
}
