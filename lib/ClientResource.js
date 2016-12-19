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
var util = require( "util" );
var csdk = require( "./csdk" );
var payload = require( "./payload" );
var handles = require( "./ClientHandles" );
var EventEmitter = require( "events" );
var listenerCount = require( "./listenerCount" );

var ClientResource = function ClientResource( init, forceNew ) {
	var result;

	// If ClientResource() was called without a constructor, we recycle the ClientResource passed
	// in as @init, but only if @forceNew is falsy.
	if ( !( this instanceof ClientResource ) ) {
		return ( ( init instanceof ClientResource ) && !forceNew ) ?
			init : new ClientResource( init, forceNew );
	}

	Object.defineProperty( this, "_private", { value: {} } );

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
			// ClientResource.observe() emits an error event, so this isn't silent.
			result = ClientResource.observe( this );
			if ( result instanceof Error ) {
				setImmediate( function( emitter, listener ) {
					emitter.removeListener( "update", listener );
				}, this, listener );
				this.emit( "error", result );
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
			} else {
				delete this._private.observer;
			}
		}
	} );
};

util.inherits( ClientResource, EventEmitter );

ClientResource.observe = function( resource, fulfill, reject ) {
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
};

module.exports = ClientResource;
