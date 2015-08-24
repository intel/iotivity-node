#include <nan.h>
#include "oc-platform-info.h"
#include "string-primitive.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

v8::Local<v8::Object> js_OCPlatformInfo( OCPlatformInfo *info ) {
	Local<Object> returnValue = NanNew<Object>();

	SET_STRING_IF_NOT_NULL( returnValue, info, platformID );
	SET_STRING_IF_NOT_NULL( returnValue, info, manufacturerName );
	SET_STRING_IF_NOT_NULL( returnValue, info, manufacturerUrl );
	SET_STRING_IF_NOT_NULL( returnValue, info, modelNumber );
	SET_STRING_IF_NOT_NULL( returnValue, info, dateOfManufacture );
	SET_STRING_IF_NOT_NULL( returnValue, info, platformVersion );
	SET_STRING_IF_NOT_NULL( returnValue, info, operatingSystemVersion );
	SET_STRING_IF_NOT_NULL( returnValue, info, hardwareVersion );
	SET_STRING_IF_NOT_NULL( returnValue, info, firmwareVersion );
	SET_STRING_IF_NOT_NULL( returnValue, info, supportUrl );
	SET_STRING_IF_NOT_NULL( returnValue, info, systemTime );

	return returnValue;
}

void c_OCPlatformInfoFreeMembers( OCPlatformInfo *info ) {
	if ( info->platformID ) {
		free( info->platformID );
	}
	if ( info->manufacturerName ) {
		free( info->manufacturerName );
	}
	if ( info->manufacturerUrl ) {
		free( info->manufacturerUrl );
	}
	if ( info->modelNumber ) {
		free( info->modelNumber );
	}
	if ( info->dateOfManufacture ) {
		free( info->dateOfManufacture );
	}
	if ( info->platformVersion ) {
		free( info->platformVersion );
	}
	if ( info->operatingSystemVersion ) {
		free( info->operatingSystemVersion );
	}
	if ( info->hardwareVersion ) {
		free( info->hardwareVersion );
	}
	if ( info->firmwareVersion ) {
		free( info->firmwareVersion );
	}
	if ( info->supportUrl ) {
		free( info->supportUrl );
	}
	if ( info->systemTime ) {
		free( info->systemTime );
	}
}

// The macro below makes use of variables defined inside c_OCPlatformInfo
#define C_PLATFORM_INFO_MEMBER( platformInfo, memberName ) \
	if ( keepGoing && (platformInfo)->Has( NanNew<String>( #memberName ) ) ) { \
		Local<Value> memberName = platformInfo->Get( NanNew<String>( #memberName ) ); \
		if ( !memberName->IsString() ) { \
			NanThrowTypeError( "platformInfo." #memberName " must be a string if it is present" ); \
			keepGoing = false; \
		} else if ( !c_StringNew( memberName->ToString(), &( local.memberName ) ) ) { \
			keepGoing = false; \
		} \
	}

bool c_OCPlatformInfo( Local<Object> platformInfo, OCPlatformInfo *info ) {
	OCPlatformInfo local = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	bool keepGoing = true;

	C_PLATFORM_INFO_MEMBER( platformInfo, platformID );
	C_PLATFORM_INFO_MEMBER( platformInfo, manufacturerName );
	C_PLATFORM_INFO_MEMBER( platformInfo, manufacturerUrl );
	C_PLATFORM_INFO_MEMBER( platformInfo, modelNumber );
	C_PLATFORM_INFO_MEMBER( platformInfo, dateOfManufacture );
	C_PLATFORM_INFO_MEMBER( platformInfo, platformVersion );
	C_PLATFORM_INFO_MEMBER( platformInfo, operatingSystemVersion );
	C_PLATFORM_INFO_MEMBER( platformInfo, hardwareVersion );
	C_PLATFORM_INFO_MEMBER( platformInfo, firmwareVersion );
	C_PLATFORM_INFO_MEMBER( platformInfo, supportUrl );
	C_PLATFORM_INFO_MEMBER( platformInfo, systemTime );

	if ( keepGoing ) {
		memcpy( info, &local, sizeof( OCPlatformInfo ) );
	} else {
		c_OCPlatformInfoFreeMembers( &local );
	}
	return keepGoing;
}
