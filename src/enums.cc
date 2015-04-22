#include <node.h>
#include <v8.h>

extern "C" {

#include <ocsocket.h>
#include <ocstack.h>

}

using namespace v8;

Local<Object> bind_OC_SOCKET_OPTION( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_SOCKET_NOOPTION" ),
		Number::New( isolate, OC_SOCKET_NOOPTION ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_SOCKET_REUSEADDR" ),
		Number::New( isolate, OC_SOCKET_REUSEADDR ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCEntityHandlerFlag( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_INIT_FLAG" ),
		Number::New( isolate, OC_INIT_FLAG ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REQUEST_FLAG" ),
		Number::New( isolate, OC_REQUEST_FLAG ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_OBSERVE_FLAG" ),
		Number::New( isolate, OC_OBSERVE_FLAG ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCEntityHandlerResult( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_OK" ),
		Number::New( isolate, OC_EH_OK ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_ERROR" ),
		Number::New( isolate, OC_EH_ERROR ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_RESOURCE_CREATED" ),
		Number::New( isolate, OC_EH_RESOURCE_CREATED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_RESOURCE_DELETED" ),
		Number::New( isolate, OC_EH_RESOURCE_DELETED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_SLOW" ),
		Number::New( isolate, OC_EH_SLOW ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_EH_FORBIDDEN" ),
		Number::New( isolate, OC_EH_FORBIDDEN ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCMethod( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_NOMETHOD" ),
		Number::New( isolate, OC_REST_NOMETHOD ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_GET" ),
		Number::New( isolate, OC_REST_GET ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_PUT" ),
		Number::New( isolate, OC_REST_PUT ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_POST" ),
		Number::New( isolate, OC_REST_POST ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_DELETE" ),
		Number::New( isolate, OC_REST_DELETE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_OBSERVE" ),
		Number::New( isolate, OC_REST_OBSERVE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_OBSERVE_ALL" ),
		Number::New( isolate, OC_REST_OBSERVE_ALL ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_CANCEL_OBSERVE" ),
		Number::New( isolate, OC_REST_CANCEL_OBSERVE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_REST_PRESENCE" ),
		Number::New( isolate, OC_REST_PRESENCE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCMode( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_CLIENT" ),
		Number::New( isolate, OC_CLIENT ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_SERVER" ),
		Number::New( isolate, OC_SERVER ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_CLIENT_SERVER" ),
		Number::New( isolate, OC_CLIENT_SERVER ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCObserveAction( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_OBSERVE_REGISTER" ),
		Number::New( isolate, OC_OBSERVE_REGISTER ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_OBSERVE_DEREGISTER" ),
		Number::New( isolate, OC_OBSERVE_DEREGISTER ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_OBSERVE_NO_OPTION" ),
		Number::New( isolate, OC_OBSERVE_NO_OPTION ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCQualityOfService( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_LOW_QOS" ),
		Number::New( isolate, OC_LOW_QOS ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_MEDIUM_QOS" ),
		Number::New( isolate, OC_MEDIUM_QOS ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_HIGH_QOS" ),
		Number::New( isolate, OC_HIGH_QOS ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_NA_QOS" ),
		Number::New( isolate, OC_NA_QOS ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCResourceProperty( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_ACTIVE" ),
		Number::New( isolate, OC_ACTIVE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_DISCOVERABLE" ),
		Number::New( isolate, OC_DISCOVERABLE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_OBSERVABLE" ),
		Number::New( isolate, OC_OBSERVABLE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_SLOW" ),
		Number::New( isolate, OC_SLOW ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_SECURE" ),
		Number::New( isolate, OC_SECURE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCStackApplicationResult( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_DELETE_TRANSACTION" ),
		Number::New( isolate, OC_STACK_DELETE_TRANSACTION ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_KEEP_TRANSACTION" ),
		Number::New( isolate, OC_STACK_KEEP_TRANSACTION ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCStackResult( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_OK" ),
		Number::New( isolate, OC_STACK_OK ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_RESOURCE_CREATED" ),
		Number::New( isolate, OC_STACK_RESOURCE_CREATED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_RESOURCE_DELETED" ),
		Number::New( isolate, OC_STACK_RESOURCE_DELETED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_CONTINUE" ),
		Number::New( isolate, OC_STACK_CONTINUE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_URI" ),
		Number::New( isolate, OC_STACK_INVALID_URI ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_QUERY" ),
		Number::New( isolate, OC_STACK_INVALID_QUERY ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_IP" ),
		Number::New( isolate, OC_STACK_INVALID_IP ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_PORT" ),
		Number::New( isolate, OC_STACK_INVALID_PORT ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_CALLBACK" ),
		Number::New( isolate, OC_STACK_INVALID_CALLBACK ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_METHOD" ),
		Number::New( isolate, OC_STACK_INVALID_METHOD ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_PARAM" ),
		Number::New( isolate, OC_STACK_INVALID_PARAM ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_OBSERVE_PARAM" ),
		Number::New( isolate, OC_STACK_INVALID_OBSERVE_PARAM ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_NO_MEMORY" ),
		Number::New( isolate, OC_STACK_NO_MEMORY ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_COMM_ERROR" ),
		Number::New( isolate, OC_STACK_COMM_ERROR ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_NOTIMPL" ),
		Number::New( isolate, OC_STACK_NOTIMPL ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_NO_RESOURCE" ),
		Number::New( isolate, OC_STACK_NO_RESOURCE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_RESOURCE_ERROR" ),
		Number::New( isolate, OC_STACK_RESOURCE_ERROR ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_SLOW_RESOURCE" ),
		Number::New( isolate, OC_STACK_SLOW_RESOURCE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_NO_OBSERVERS" ),
		Number::New( isolate, OC_STACK_NO_OBSERVERS ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_OBSERVER_NOT_FOUND" ),
		Number::New( isolate, OC_STACK_OBSERVER_NOT_FOUND ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_VIRTUAL_DO_NOT_HANDLE" ),
		Number::New( isolate, OC_STACK_VIRTUAL_DO_NOT_HANDLE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_OPTION" ),
		Number::New( isolate, OC_STACK_INVALID_OPTION ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_MALFORMED_RESPONSE" ),
		Number::New( isolate, OC_STACK_MALFORMED_RESPONSE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_PERSISTENT_BUFFER_REQUIRED" ),
		Number::New( isolate, OC_STACK_PERSISTENT_BUFFER_REQUIRED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_REQUEST_HANDLE" ),
		Number::New( isolate, OC_STACK_INVALID_REQUEST_HANDLE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_INVALID_DEVICE_INFO" ),
		Number::New( isolate, OC_STACK_INVALID_DEVICE_INFO ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_PRESENCE_STOPPED" ),
		Number::New( isolate, OC_STACK_PRESENCE_STOPPED ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_PRESENCE_TIMEOUT" ),
		Number::New( isolate, OC_STACK_PRESENCE_TIMEOUT ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_PRESENCE_DO_NOT_HANDLE" ),
		Number::New( isolate, OC_STACK_PRESENCE_DO_NOT_HANDLE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_STACK_ERROR" ),
		Number::New( isolate, OC_STACK_ERROR ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCTransportProtocolID( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_INVALID_ID" ),
		Number::New( isolate, OC_INVALID_ID ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_COAP_ID" ),
		Number::New( isolate, OC_COAP_ID ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

Local<Object> bind_OCVirtualResources( Isolate* isolate ) {
	Local<Object> obj = Object::New( isolate );

	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_WELL_KNOWN_URI" ),
		Number::New( isolate, OC_WELL_KNOWN_URI ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_DEVICE_URI" ),
		Number::New( isolate, OC_DEVICE_URI ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_RESOURCE_TYPES_URI" ),
		Number::New( isolate, OC_RESOURCE_TYPES_URI ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_PRESENCE" ),
		Number::New( isolate, OC_PRESENCE ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );
	obj->ForceSet(
		String::NewFromUtf8( isolate, "OC_MAX_VIRTUAL_RESOURCES" ),
		Number::New( isolate, OC_MAX_VIRTUAL_RESOURCES ),
		static_cast<PropertyAttribute>( ReadOnly || DontDelete ) );

	return obj;
}

void InitEnums( Handle<Object> exports ) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	exports->Set( String::NewFromUtf8( isolate, "OC_SOCKET_OPTION" ),
		bind_OC_SOCKET_OPTION( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCEntityHandlerFlag" ),
		bind_OCEntityHandlerFlag( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCEntityHandlerResult" ),
		bind_OCEntityHandlerResult( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCMethod" ),
		bind_OCMethod( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCMode" ),
		bind_OCMode( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCObserveAction" ),
		bind_OCObserveAction( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCQualityOfService" ),
		bind_OCObserveAction( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCResourceProperty" ),
		bind_OCResourceProperty( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCStackApplicationResult" ),
		bind_OCStackApplicationResult( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCStackResult" ),
		bind_OCStackResult( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCTransportProtocolID" ),
		bind_OCTransportProtocolID( isolate ) );
	exports->Set( String::NewFromUtf8( isolate, "OCVirtualResources" ),
		bind_OCVirtualResources( isolate ) );
	
}
