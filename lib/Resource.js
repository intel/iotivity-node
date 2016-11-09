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
var EventEmitter = require( "events" ).EventEmitter;
var handles = require( "./ClientHandles" );
var listenerCount = require( "./listenerCount" );
var payload = require( "./payload" );
var csdk = require( "./csdk" );

var Resource = function Resource( init, forceNew ) {
	var result;

	// If Resource() was called without a constructor, we recycle the Resource passed in as @init,
	// but only if @forceNew is falsy.
	if ( !( this instanceof Resource ) ) {
		return ( ( init instanceof Resource ) && !forceNew ) ?
			init : new Resource( init, forceNew );
	}

	Object.defineProperty( this, "_private", { writable: true, value: {} } );

	// Grab the relevant properties from the init, ignoring all others
	_.extend( this,
		{ properties: {} },
		_.pick( init, [
			"resourcePath", "deviceId", "interfaces", "resourceTypes",  "properties",
			"discoverable", "slow", "secure", "observable", "active"
		] ) );


	this.on( "newListener", function( eventName, listener ) {
		if ( eventName === "update" && !this._private.observer ) {

			// If we fail to observe, we remove the listener immediately after it was added.
			// Resource.observe() emits an error event, so this isn't silent.
			if ( Resource.observe( this ) instanceof Error ) {
				setImmediate( function( emitter, listener ) {
					emitter.removeListener( "update", listener );
				}, this, listener );
			}
		}
	} );

	this.on( "removeListener", function() {
		if ( listenerCount( this, "update" ) === 0 && this._private.observer ) {
			result = handles.replace( {
				method: "OC_REST_OBSERVE",
				requestUri: this.resourcePath,
				query: this._private.query,
				deviceId: this.deviceId
			}, this._private.observer, "remove" );
			if ( result instanceof Error ) {
				this.emit( "error", result );
			}
		}
	} );
};
require( "util" ).inherits( Resource, EventEmitter );

module.exports = _.extend( Resource, {
	observe: function( resource, fulfill, reject ) {
		var result = true;

		// Influences the behaviour of updateResource. If set to true, it causes
		// updateResource to call fulfill/reject when it is first called.
		var needsResolve = ( fulfill && reject );

		// Function called for both observation and retrieval. needsResolve tells us
		// whether there is an outstanding promise.
		var updateResource = function( response ) {
			var properties;

			if ( response.result !== csdk.OCStackResult.OC_STACK_OK ) {
				resource.emit( "error", _.extend( new Error( "Server responded with error" ), {
					response: response
				} ) );
			}

			properties = response.payload ?
				payload.repPayloadToObject( response.payload ) : {};
			if ( properties instanceof Error ) {
				resource.emit( "error", properties );
				if ( needsResolve ) {
					reject( properties );
				}
			} else {
				_.extend( resource.properties, properties );
				resource.emit( "update", resource );
				if ( needsResolve ) {
					fulfill( resource );
				}
			}

			// Once we've resolved the promise we set this to false to avoid resolving
			// it multiple times.
			needsResolve = false;
		};

		if ( !resource._private.observer ) {
			result = handles.replace( {
				method: "OC_REST_OBSERVE",
				requestUri: resource.resourcePath,
				query: resource._private.query,
				deviceId: resource.deviceId
			}, updateResource );

			// If there was an error opening the handle, we reject the promise.
			if ( result instanceof Error ) {
				if ( needsResolve ) {
					reject( result );
				}
				resource.emit( "error", result );
			} else {
				resource._private.observer = updateResource;
			}
		}

		// If there was already a handle attached, then this handler was
		// added to the existing ones, but no new information will arrive
		// from the resource, so we cannot call fulfill() or reject() in time.
		if ( result === true ) {
			needsResolve = false;
		}

		return result;
	},
	setServerResource: function( resource ) {
		Object.defineProperty( resource, "resourcePath", {
			enumerable: true,
			get: function() {
				return csdk.OCGetResourceUri( this._private.handle );
			}
		} );
		Object.defineProperty( resource, "slow", {
			enumerable: true,
			get: function() {
				return !!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_SLOW );
			}
		} );
		Object.defineProperty( resource, "active", {
			enumerable: true,
			get: function() {
				return !!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_ACTIVE );
			}
		} );
		Object.defineProperty( resource, "secure", {
			enumerable: true,
			get: function() {
				return !!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_SECURE );
			}
		} );
		Object.defineProperty( resource, "observable", {
			enumerable: true,
			get: function() {
				return !!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_OBSERVABLE );
			}
		} );
		Object.defineProperty( resource, "discoverable", {
			enumerable: true,
			get: function() {
				return !!( csdk.OCGetResourceProperties( this._private.handle ) &
					csdk.OCResourceProperty.OC_DISCOVERABLE );
			}
		} );
	}
} );
