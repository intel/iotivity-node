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
var utils = require( "../../assert-to-console" );
var oic = require( "../../../index" )( "client" );
var uuid = process.argv[ 2 ];
var expectedDeviceInfo = {

	// The uuid field will be set to the deviceId when we know the deviceId
	name: "API Device Info Server " + uuid,
	coreSpecVersion: "core.1.0.0"
};
var expectedPlatformInfo = {
	id: "platform " + uuid,
	manufacturerName: "Random",
	manufactureDate: 1299752880000,
	manufacturerUrl: "http://example.com/",
	model: "model " + uuid,
	osVersion: "osVersion " + uuid,
	platformVersion: "platformVersion " + uuid,
	firmwareVersion: "firmwareVersion " + uuid,
	supportUrl: "supportUrl:" + uuid
};

console.log( JSON.stringify( { assertionCount: 4 } ) );

new Promise( function findTheDeviceId( fulfill, reject ) {
	var teardown;
	var resourcefound = function( event ) {
		teardown( null, event.resource.id.deviceId );
	};
	teardown = function( error, deviceId ) {
		oic.removeEventListener( "resourcefound", resourcefound );
		if ( error ) {
			reject( error );
		} else {

			// Complete the expectedDeviceInfo structure
			expectedDeviceInfo.uuid = deviceId;
			fulfill( deviceId );
		}
	};
	oic.addEventListener( "resourcefound", resourcefound );
	oic.findResources( { resourcePath: "/a/" + uuid } ).catch( teardown );
} ).then( function getDeviceInfo( deviceId ) {
	return oic.getDeviceInfo( deviceId ).then( function assertDeviceInfo( deviceInfo ) {
		utils.assert( "deepEqual", deviceInfo, expectedDeviceInfo,
			"Client: The retrieved device information is as expected" );
		return deviceId;
	} );
} ).then( function getPlatformInfo( deviceId ) {
	return oic.getPlatformInfo( deviceId ).then( function assertPlatformInfo( platformInfo ) {

		// Convert the manufactureDate to a timestamp for unambiguous comparison
		utils.assert( "deepEqual", _.extend( platformInfo, {
			manufactureDate: new Date( platformInfo.manufactureDate ).getTime()
		} ), expectedPlatformInfo, "Client: The retrieved platform information is as expected" );
	} );
} ).then( function findDevices() {
	return new Promise( function( fulfill, reject ) {
		var devicefound,
			teardown = function( error, remoteDevice ) {
				oic.removeEventListener( "devicefound", devicefound );
				if ( error ) {
					reject( error );
				} else {
					fulfill( remoteDevice );
				}
			};
		devicefound = function( event ) {
			if ( event.device.uuid === expectedDeviceInfo.uuid ) {
				teardown( null, event.device );
			}
		};
		oic.addEventListener( "devicefound", devicefound );
		oic.findDevices().catch( teardown );
	} ).then( function assertDiscoveredDeviceInfo( deviceInfo ) {
		utils.assert( "deepEqual", deviceInfo, expectedDeviceInfo,
			"Client: The discovered device information is as expected" );
	} );
} ).then( function findPlatforms() {
	return new Promise( function( fulfill, reject ) {
		var platformfound,
			teardown = function( error, remotePlatform ) {
				oic.removeEventListener( "platformfound", platformfound );
				if ( error ) {
					reject( error );
				} else {
					fulfill( remotePlatform );
				}
			};
		platformfound = function( event ) {
			if ( event.platform.id === expectedPlatformInfo.id ) {
				teardown( null, event.platform );
			}
		};
		oic.addEventListener( "platformfound", platformfound );
		oic.findPlatforms().catch( teardown );
	} ).then( function assertDiscoveredPlatformInfo( platformInfo ) {

		// Convert the manufactureDate to a timestamp for unambiguous comparison
		utils.assert( "deepEqual", _.extend( platformInfo, {
			manufactureDate: new Date( platformInfo.manufactureDate ).getTime()
		} ), expectedPlatformInfo, "Client: The discovered platform information is as expected" );
	} );
} ).then(
	function testIsDone() {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	},
	function catchError( error ) {
		utils.die( "Client: Failed with error " + error.message + " and result " + error.result );
	} );
