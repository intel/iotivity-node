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
var resolver = require( "./Resolver" );
var querystring = require( "querystring" );
var listenerCount = require( "./listenerCount" );

var ClientHandles = function Handles() {
	if ( !( this instanceof ClientHandles ) ) {
		return new ClientHandles();
	}

	// The key is an object with options passed to open() in JSON representation
	// The value is an object containing the native handle and the key as an object
	this._handles = {};
};

require( "util" ).inherits( ClientHandles, require( "events" ).EventEmitter );

_.extend( ClientHandles.prototype, {

	// options: {
	//   method: string
	//   requestUri: string
	//   destination: string (deviceId)
	// }
	replace: function( options, handler, operation ) {
		var eventName;
		var result;
		var destination = null;
		var handleReceptacle = {};

		// Find the handle corresponding to options. If present, also overwrite options with the
		// key we've found, because the key is deeply equal to options, but we need the exact
		// object that's been used as the key so we can overwrite it in the hash.
		var handle = ( _.find( this._handles, function( value, key ) {
			if ( _.isEqual( value.key, options ) ) {
				eventName = key;
				return true;
			}
			return false;
		} ) || {} ).handle;

		if ( handle ) {
			if ( operation === "remove" ) {
				this.removeListener( eventName, handler );
				if ( listenerCount( this, eventName ) > 0 ) {
					return true;
				}
			}

			result =
				csdk.OCCancel( handle, csdk.OCQualityOfService.OC_HIGH_QOS, null, 0 );
			if ( result === csdk.OCStackResult.OC_STACK_OK ) {
				handle = undefined;
			} else {
				return _.extend( new Error( "Failed to cancel handle" ), {
					result: result,
					options: options
				} );
			}
		} else if ( operation === "remove" ) {
			return _.extend( new Error( "Native handler not found" ), {
				options: options
			} );
		}

		if ( operation !== "remove" ) {
			eventName = JSON.stringify( options );

			if ( options.deviceId ) {
				destination = resolver.get( options.deviceId );
				if ( destination instanceof Error ) {
					return destination;
				}
			}

			result = csdk.OCDoResource( handleReceptacle,
				csdk.OCMethod[ options.method ],
				options.requestUri +
					( options.query ? ( "?" + querystring.stringify( options.query ) ) : "" ),
				destination,
				null,
				csdk.OCConnectivityType.CT_DEFAULT,
				csdk.OCQualityOfService.OC_HIGH_QOS,
				_.bind( function( nativeHandle, response ) {
					this.emit( eventName, response );
					return csdk.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;
				}, this ),
				null, 0 );
			if ( result === csdk.OCStackResult.OC_STACK_OK ) {
				this.on( eventName, handler );
				handle = handleReceptacle.handle;
			} else {
				return _.extend( new Error( "Request failed" ), {
					result: result,
					options: options
				} );
			}
		}

		if ( handle ) {
			this._handles[ eventName ] = { handle: handle, key: options };
		} else {
			delete this._handles[ eventName ];
		}
	}
} );

module.exports = ClientHandles();
