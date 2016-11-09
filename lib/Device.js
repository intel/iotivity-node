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

var _ = require( "lodash" );
var csdk = require( "./csdk" );
var BackedObject = require( "./BackedObject" );

var ocf = {};

BackedObject.attach( ocf, "platform", {
	osVersion: null,
	model: null,
	manufacturerName: "default",
	manufacturerURL: null,
	manufacturerDate: null,
	platformVersion: null,
	firmwareVersion: null,
	supportURL: null
}, function commitPlatformInfo( info ) {
	var platformInfo =  _.extend( {

		// Required fields
		platformID: info.id,
		manufacturerName: ( function validateManufacturerName( name ) {
			if ( typeof name !== "string" || !name ) {
				throw new Error( "manufacturer name must be a non-empty string" );
			}
			if ( name.length > csdk.MAX_MANUFACTURER_NAME_LENGTH ) {
				throw new Error( "manufacturer name length must not exceed " +
					csdk.MAX_MANUFACTURER_NAME_LENGTH + " characters" );
			}
			return name;
		} )( info.manufacturerName )

	// NOTE: systemTime and hardwareVersion are absent from the spec
	},

		// Optional fields
		typeof info.manufacturerURL === "string" && info.manufacturerURL ?
			{ manufacturerUrl: ( function validateManufacturerUrl( url ) {
					if ( url && url.length > csdk.MAX_MANUFACTURER_URL_LENGTH ) {
						throw new Error( "manufacturer URL length must not exceed " +
							csdk.MAX_MANUFACTURER_URL_LENGTH + " characters" );
					}
					return url;
			} )( info.manufacturerURL ) } : {},
		typeof info.model === "string" && info.model ?
			{ modelNumber: info.model } : {},
		typeof info.platformVersion === "string" && info.platformVersion ?
			{ platformVersion: info.platformVersion } : {},
		typeof info.osVersion === "string" && info.osVersion ?
			{ operatingSystemVersion: info.osVersion } : {},
		typeof info.firmwareVersion === "string" && info.firmwareVersion ?
			{ firmwareVersion: info.firmwareVersion } : {},
		typeof info.supportURL === "string" && info.supportURL ?
			{ supportUrl: info.supportURL } : {},
		"manufactureDate" in info ? { dateOfManufacture: info.manufactureDate.toString() } : {}
	);

	var result = csdk.OCSetPlatformInfo( platformInfo );

	if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
		return _.extend( new Error( "Set platform info failed" ), {
			result: result
		} );
	}
}, function platformInfoWasSet( newValue ) {
	Object.defineProperty( newValue, "id", {
		enumerable: true,
		get: _.bindKey( csdk, "OCGetServerInstanceIDString" )
	} );
} );

BackedObject.attach( ocf, "device", {
	url: null,
	name: "default",
	dataModels: [ "res.1.0.0" ],
	coreSpecVersion: "1.0.0"
}, function commitDeviceInfo( info ) {
	var result = csdk.OCSetDeviceInfo( {
		specVersion: info.coreSpecVersion,
		deviceName: info.name,
		dataModelVersions: info.dataModels,
		types: []
	} );

	if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
		throw _.extend( new Error( "Set device info failed" ), {
			result: result
		} );
	}
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
