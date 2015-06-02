#include <node.h>
#include <nan.h>

extern "C" {
#include <ocstack.h>
}

using namespace v8;

static Local<Object> bind_OCEntityHandlerFlag() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_INIT_FLAG"),
                        NanNew<Number>(OC_INIT_FLAG),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REQUEST_FLAG"),
                        NanNew<Number>(OC_REQUEST_FLAG),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_OBSERVE_FLAG"),
                        NanNew<Number>(OC_OBSERVE_FLAG),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCEntityHandlerResult() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_EH_OK"), NanNew<Number>(OC_EH_OK),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EH_ERROR"),
                        NanNew<Number>(OC_EH_ERROR),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EH_RESOURCE_CREATED"),
                        NanNew<Number>(OC_EH_RESOURCE_CREATED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EH_RESOURCE_DELETED"),
                        NanNew<Number>(OC_EH_RESOURCE_DELETED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EH_SLOW"),
                        NanNew<Number>(OC_EH_SLOW),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EH_FORBIDDEN"),
                        NanNew<Number>(OC_EH_FORBIDDEN),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCMethod() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_REST_NOMETHOD"),
                        NanNew<Number>(OC_REST_NOMETHOD),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_GET"),
                        NanNew<Number>(OC_REST_GET),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_PUT"),
                        NanNew<Number>(OC_REST_PUT),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_POST"),
                        NanNew<Number>(OC_REST_POST),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_DELETE"),
                        NanNew<Number>(OC_REST_DELETE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_OBSERVE"),
                        NanNew<Number>(OC_REST_OBSERVE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_OBSERVE_ALL"),
                        NanNew<Number>(OC_REST_OBSERVE_ALL),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_CANCEL_OBSERVE"),
                        NanNew<Number>(OC_REST_CANCEL_OBSERVE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_REST_PRESENCE"),
                        NanNew<Number>(OC_REST_PRESENCE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCMode() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_CLIENT"), NanNew<Number>(OC_CLIENT),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_SERVER"), NanNew<Number>(OC_SERVER),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_CLIENT_SERVER"),
                        NanNew<Number>(OC_CLIENT_SERVER),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCObserveAction() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_OBSERVE_REGISTER"),
                        NanNew<Number>(OC_OBSERVE_REGISTER),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_OBSERVE_DEREGISTER"),
                        NanNew<Number>(OC_OBSERVE_DEREGISTER),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_OBSERVE_NO_OPTION"),
                        NanNew<Number>(OC_OBSERVE_NO_OPTION),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCQualityOfService() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_LOW_QOS"),
                        NanNew<Number>(OC_LOW_QOS),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_MEDIUM_QOS"),
                        NanNew<Number>(OC_MEDIUM_QOS),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_HIGH_QOS"),
                        NanNew<Number>(OC_HIGH_QOS),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_NA_QOS"), NanNew<Number>(OC_NA_QOS),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCResourceProperty() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_ACTIVE"), NanNew<Number>(OC_ACTIVE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_DISCOVERABLE"),
                        NanNew<Number>(OC_DISCOVERABLE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_OBSERVABLE"),
                        NanNew<Number>(OC_OBSERVABLE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_SLOW"), NanNew<Number>(OC_SLOW),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_SECURE"), NanNew<Number>(OC_SECURE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCStackApplicationResult() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_STACK_DELETE_TRANSACTION"),
                        NanNew<Number>(OC_STACK_DELETE_TRANSACTION),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_KEEP_TRANSACTION"),
                        NanNew<Number>(OC_STACK_KEEP_TRANSACTION),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCStackResult() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_STACK_OK"),
                        NanNew<Number>(OC_STACK_OK),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_RESOURCE_CREATED"),
                        NanNew<Number>(OC_STACK_RESOURCE_CREATED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_RESOURCE_DELETED"),
                        NanNew<Number>(OC_STACK_RESOURCE_DELETED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_CONTINUE"),
                        NanNew<Number>(OC_STACK_CONTINUE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_URI"),
                        NanNew<Number>(OC_STACK_INVALID_URI),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_QUERY"),
                        NanNew<Number>(OC_STACK_INVALID_QUERY),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_IP"),
                        NanNew<Number>(OC_STACK_INVALID_IP),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_PORT"),
                        NanNew<Number>(OC_STACK_INVALID_PORT),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_CALLBACK"),
                        NanNew<Number>(OC_STACK_INVALID_CALLBACK),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_METHOD"),
                        NanNew<Number>(OC_STACK_INVALID_METHOD),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_PARAM"),
                        NanNew<Number>(OC_STACK_INVALID_PARAM),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_OBSERVE_PARAM"),
                        NanNew<Number>(OC_STACK_INVALID_OBSERVE_PARAM),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_NO_MEMORY"),
                        NanNew<Number>(OC_STACK_NO_MEMORY),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_COMM_ERROR"),
                        NanNew<Number>(OC_STACK_COMM_ERROR),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_NOTIMPL"),
                        NanNew<Number>(OC_STACK_NOTIMPL),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_NO_RESOURCE"),
                        NanNew<Number>(OC_STACK_NO_RESOURCE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_RESOURCE_ERROR"),
                        NanNew<Number>(OC_STACK_RESOURCE_ERROR),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_SLOW_RESOURCE"),
                        NanNew<Number>(OC_STACK_SLOW_RESOURCE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_NO_OBSERVERS"),
                        NanNew<Number>(OC_STACK_NO_OBSERVERS),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_OBSERVER_NOT_FOUND"),
                        NanNew<Number>(OC_STACK_OBSERVER_NOT_FOUND),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_VIRTUAL_DO_NOT_HANDLE"),
                        NanNew<Number>(OC_STACK_VIRTUAL_DO_NOT_HANDLE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_OPTION"),
                        NanNew<Number>(OC_STACK_INVALID_OPTION),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_MALFORMED_RESPONSE"),
                        NanNew<Number>(OC_STACK_MALFORMED_RESPONSE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_PERSISTENT_BUFFER_REQUIRED"),
                        NanNew<Number>(OC_STACK_PERSISTENT_BUFFER_REQUIRED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_REQUEST_HANDLE"),
                        NanNew<Number>(OC_STACK_INVALID_REQUEST_HANDLE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_INVALID_DEVICE_INFO"),
                        NanNew<Number>(OC_STACK_INVALID_DEVICE_INFO),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_PRESENCE_STOPPED"),
                        NanNew<Number>(OC_STACK_PRESENCE_STOPPED),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_PRESENCE_TIMEOUT"),
                        NanNew<Number>(OC_STACK_PRESENCE_TIMEOUT),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_PRESENCE_DO_NOT_HANDLE"),
                        NanNew<Number>(OC_STACK_PRESENCE_DO_NOT_HANDLE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_STACK_ERROR"),
                        NanNew<Number>(OC_STACK_ERROR),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCTransportProtocolID() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_INVALID_ID"),
                        NanNew<Number>(OC_INVALID_ID),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_COAP_ID"),
                        NanNew<Number>(OC_COAP_ID),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCVirtualResources() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_WELL_KNOWN_URI"),
                        NanNew<Number>(OC_WELL_KNOWN_URI),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_DEVICE_URI"),
                        NanNew<Number>(OC_DEVICE_URI),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_RESOURCE_TYPES_URI"),
                        NanNew<Number>(OC_RESOURCE_TYPES_URI),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_PRESENCE"),
                        NanNew<Number>(OC_PRESENCE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_MAX_VIRTUAL_RESOURCES"),
                        NanNew<Number>(OC_MAX_VIRTUAL_RESOURCES),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

  return returnValue;
}

static Local<Object> bind_OCConnectivityType() {
  Local<Object> returnValue = NanNew<Object>();

  returnValue->ForceSet(NanNew<String>("OC_ETHERNET"),
                        NanNew<Number>(OC_ETHERNET),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_WIFI"), NanNew<Number>(OC_WIFI),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_EDR"), NanNew<Number>(OC_EDR),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_LE"), NanNew<Number>(OC_LE),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));
  returnValue->ForceSet(NanNew<String>("OC_ALL"), NanNew<Number>(OC_ALL),
                        static_cast<PropertyAttribute>(ReadOnly || DontDelete));

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
