// Copyright 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var _ = {
	extend: require( "lodash.assignin" ),
	bindKey: require( "lodash.bindkey" )
};
var csdk = require( "./csdk" );
var BackedObject = require( "./BackedObject" );

var ocf = {};

function maybeAddString( value, destination, property ) {
	if ( typeof value === "string" && value ) {
		destination[ property ] = value;
	}
}

function commitInfo( prefix, info, destination ) {
	var index, result;

	for ( index in info ) {
		result = csdk.OCSetPropertyValue( destination, index, info[ index ] );
		if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
			throw _.extend( new Error( "Set " + prefix + " info failed" ), {
				property: index,
				result: result
			} );
		}
	}
}

BackedObject.attach( ocf, "platform", {
	osVersion: null,
	model: null,
	manufacturerName: "default",
	manufacturerURL: null,
	platformVersion: null,
	firmwareVersion: null,
	supportURL: null
}, function commitPlatformInfo( info ) {
	var platformInfo = {};

	// Required fields
	platformInfo[ csdk.OC_RSRVD_MFG_NAME ] = ( function validateManufacturerName( name ) {
		if ( typeof name !== "string" || !name ) {
			throw new Error( "manufacturer name must be a non-empty string" );
		}
		if ( name.length > csdk.MAX_MANUFACTURER_NAME_LENGTH ) {
			throw new Error( "manufacturer name length must not exceed " +
				csdk.MAX_MANUFACTURER_NAME_LENGTH + " characters" );
		}
		return name;
	} )( info.manufacturerName );

	// Optional fields
	if ( typeof info.manufacturerURL === "string" && info.manufacturerURL ) {
		platformInfo[ csdk.OC_RSRVD_MFG_URL ] = ( function validateManufacturerUrl( url ) {
			if ( url && url.length > csdk.MAX_MANUFACTURER_URL_LENGTH ) {
				throw new Error( "manufacturer URL length must not exceed " +
					csdk.MAX_MANUFACTURER_URL_LENGTH + " characters" );
			}
			return url;
		} )( info.manufacturerURL );
	}

	if ( "manufactureDate" in info ) {
		platformInfo[ csdk.OC_RSRVD_MFG_DATE ] = info.manufactureDate;
	}
	if ( "systemTime" in info ) {
		platformInfo[ csdk.OC_RSRVD_SYSTEM_TIME ] = info.systemTime;
	}

	maybeAddString( info.model, platformInfo, csdk.OC_RSRVD_MODEL_NUM );
	maybeAddString( info.platformVersion, platformInfo, csdk.OC_RSRVD_PLATFORM_VERSION );
	maybeAddString( info.osVersion, platformInfo, csdk.OC_RSRVD_OS_VERSION );
	maybeAddString( info.firmwareVersion, platformInfo, csdk.OC_RSRVD_FIRMWARE_VERSION );
	maybeAddString( info.supportURL, platformInfo, csdk.OC_RSRVD_SUPPORT_URL );
	maybeAddString( info.hardwareVersion, platformInfo, csdk.OC_RSRVD_HARDWARE_VERSION );

	commitInfo( "platform", platformInfo, csdk.OCPayloadType.PAYLOAD_TYPE_PLATFORM );
}, function platformInfoWasSet( newValue ) {
	Object.defineProperty( newValue, "id", {
		enumerable: true,
		get: _.bindKey( csdk, "OCGetServerInstanceIDString" )
	} );
} );

BackedObject.attach( ocf, "device", {
	url: null,
	name: "default",
	dataModels: [ "res.1.1.0" ],
	coreSpecVersion: "res.1.1.0",
	types: []
}, function commitDeviceInfo( info ) {
	var nativeInfo = {};

	nativeInfo[ csdk.OC_RSRVD_SPEC_VERSION ] = info.coreSpecVersion;
	nativeInfo[ csdk.OC_RSRVD_DATA_MODEL_VERSION ] = info.dataModels.join( "," );
	maybeAddString( info.name, nativeInfo, csdk.OC_RSRVD_DEVICE_NAME );

	commitInfo( "device", nativeInfo, csdk.OCPayloadType.PAYLOAD_TYPE_DEVICE );
}, function deviceInfoWasSet( newValue ) {
	Object.defineProperty( newValue, "uuid", {
		enumerable: true,
		get: _.bindKey( csdk, "OCGetServerInstanceIDString" )
	} );
} );

Object.defineProperty( ocf, "client", {
	enumerable: true,
	value: require( "./Client" )
} );

Object.defineProperty( ocf, "server", {
	enumerable: true,
	value: require( "./Server" )
} );

module.exports = ocf;
