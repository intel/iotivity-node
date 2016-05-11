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

var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "lodash" ),
	StorageHandler = require( "./StorageHandler" ),
	utils = require( "./utils" ),
	OicDevice = function OicDevice( role ) {
		if ( !this._isOicDevice ) {
			return new OicDevice( role );
		}

		this._construct( role );
	};

_.extend( utils.makeEventEmitter( OicDevice ).prototype, {
	_construct: function( role ) {
		utils.setPrivate( this, [ "_resources", "OCProcessIntervalId", "_info" ] );

		Object.defineProperty( this, "device", {
			configurable: true,
			enumerable: true,
			get: _.bind( this._getDeviceInfo, this ),
			set: _.bind( this._setDeviceInfo, this )
		} );

		Object.defineProperty( this, "platform", {
			configurable: true,
			enumerable: true,
			get: _.bind( this._getPlatformInfo, this ),
			set: _.bind( this._setPlatformInfo, this )
		} );

		// Hash with two keys: device for device info and platform for platform info
		this._info = { device: { role: role }, platform: {} };

		this._resources = {};

		this._stopStack();
		this._startStack( role );

		this._OCProcessIntervalId = setInterval( iotivity.OCProcess, 100 );
	},
	_isOicDevice: true,

	_stopStack: function() {
		var result;

		if ( this._OCProcessIntervalId ) {
			result = iotivity.OCStop();
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				throw _.extend( new Error( "configure: OCStop() failed" ), {
					result: result
				} );
			} else {
				clearInterval( this._OCProcessIntervalId );
				this._OCProcessIntervalId = null;
			}
		}
	},

	_getDeviceInfo: function() {
		return _.extend( this._info.device, {
			uuid: iotivity.OCGetServerInstanceIDString()
		} );
	},

	_setDeviceInfo: function( info ) {
		var result = iotivity.OCSetDeviceInfo( { deviceName: info.name, types: [] } );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			throw _.extend( new Error( "Set device info failed" ), {
				result: result
			} );
		}
		_.extend( this._info.device, info );
	},

	_getPlatformInfo: function() {
		return _.extend( this._info.platform, {
			id: iotivity.OCGetServerInstanceIDString()
		} );
	},

	_setPlatformInfo: function( info ) {
		var result = iotivity.OCSetPlatformInfo( _.extend( {

			// Required fields
			platformID: info.id,
			manufacturerName: ( function validateManufacturerName( name ) {
				if ( typeof name !== "string" || !name ) {
					throw new Error( "manufacturer name must be a non-empty string" );
				}
				if ( name.length > iotivity.MAX_MANUFACTURER_NAME_LENGTH ) {
					throw new Error( "manufacturer name length must not exceed " +
						iotivity.MAX_MANUFACTURER_NAME_LENGTH + " characters" );
				}
				return name;
			} )( info.manufacturerName )

		// NOTE: systemTime and hardwareVersion are absent from the spec
		},

			// Optional fields
			typeof info.manufacturerUrl === "string" && info.manufacturerUrl ?
				{ manufacturerUrl: ( function validateManufacturerUrl( url ) {
						if ( url && url.length > iotivity.MAX_MANUFACTURER_URL_LENGTH ) {
							throw new Error( "manufacturer URL length must not exceed " +
								iotivity.MAX_MANUFACTURER_URL_LENGTH + " characters" );
						}
						return url;
				} )( info.manufacturerUrl ) } : {},
			typeof info.model === "string" && info.model ?
				{ modelNumber: info.model } : {},
			typeof info.platformVersion === "string" && info.platformVersion ?
				{ platformVersion: info.platformVersion } : {},
			typeof info.osVersion === "string" && info.osVersion ?
				{ operatingSystemVersion: info.osVersion } : {},
			typeof info.firmwareVersion === "string" && info.firmwareVersion ?
				{ firmwareVersion: info.firmwareVersion } : {},
			typeof info.supportUrl === "string" && info.supportUrl ?
				{ supportUrl: info.supportUrl } : {},
			"manufactureDate" in info ? { dateOfManufacture: info.manufactureDate.toString() } : {}
		) );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			throw _.extend( new Error( "Set platform info failed" ), {
				result: result
			} );
		}
		_.extend( this._info.platform, info );
	},

	_startStack: function( role ) {
		var result;

		if ( role !== "client" ) {
			iotivity.OCRegisterPersistentStorageHandler( new StorageHandler() );
		}

		result = iotivity.OCInit( null, 0,
			iotivity.OCMode[ role === "client" ? "OC_CLIENT" :
				role === "server" ? "OC_SERVER" : "OC_CLIENT_SERVER" ] );
		if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
			throw _.extend( new Error( "configure: OCInit() failed" ), {
				result: result
			} );
		}
	}
} );

// Extend OicDevice with client and server interfaces
require( "./OicClient" )( OicDevice.prototype );
require( "./OicServer" )( OicDevice.prototype );

module.exports = OicDevice;
