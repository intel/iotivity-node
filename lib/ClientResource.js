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
	findIndex: require( "lodash.findindex" ),
	extend: require( "lodash.assignin" ),
	pick: require( "lodash.pick" )
};
var util = require( "util" );
var csdk = require( "./csdk" );
var payload = require( "./payload" );
var handles = require( "./ClientHandles" );
var EventEmitter = require( "events" );
var listenerCount = require( "./listenerCount" );

// This function gets attached to "devicelost" in order to enable the generation of
// "_resourceslost" events which the ClientResource objects use to generate "delete" events
function devicelost() {}

function finalizeResource( resource ) {
	resource.emit( "delete", resource );
	resource.removeAllListeners( "update" );
	resource.removeAllListeners( "delete" );
}

function watchDelete( resource ) {
	var resourceslost = function( data ) {
		if ( data.device.uuid === resource.deviceId &&
				_.findIndex( data.resources, function( remainingResource ) {
					return ( remainingResource.resourcePath === resource.resourcePath );
				} ) === -1 ) {
			finalizeResource( resource );
		}
	};

	// "_resourceslost" doesn't work without "devicelost", so, if nobody is listening to
	// "_resourceslost", we need to make sure /somebody/ is listening to "devicelost" so that
	// "_resourceslost" will also be emitted.
	if ( listenerCount( ClientResource.client, "_resourceslost" ) === 0 ) {
		ClientResource.client.on( "devicelost", devicelost );
	}

	ClientResource.client.on( "_resourceslost", resourceslost );
	resource._private.resourceslost = resourceslost;
}

function unwatchDelete( resource ) {

	ClientResource.client.removeListener( "_resourceslost", resource._private.resourceslost );

	// If nobody cares about "_resourceslost", then we remove the dummy "devicelost" handler too
	if ( listenerCount( ClientResource.client, "_resourceslost" ) === 0 ) {
		ClientResource.client.removeListener( "devicelost", devicelost );
	}
}

function observeParams( resource ) {
	return _.extend( ClientResource.getDestination( resource ), {
		method: "OC_REST_OBSERVE",
		query: resource._private.query
	} );
}

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
		{
			properties: {}
		},
		_.pick( init, [
			"resourcePath", "anchor", "deviceId", "interfaces", "resourceTypes",  "properties",
			"discoverable", "slow", "secure", "observable", "active"
		] ) );

	if ( "_private" in init ) {
		this._private.origin = init._private.origin;
	}

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

		if ( eventName === "delete" && listenerCount( this, "delete" ) === 0 ) {
			watchDelete( this );
		}
	} );

	this.on( "removeListener", function( eventName ) {
		if ( eventName === "update" &&
				listenerCount( this, "update" ) === 0 &&
				this._private.observer ) {
			result = handles.replace( observeParams( this ), this._private.observer, "remove" );
			if ( result instanceof Error ) {
				this.emit( "error", result );
			} else {
				delete this._private.observer;
			}
		}
		if ( eventName === "delete" && listenerCount( this, "delete" ) === 0 ) {
			unwatchDelete( this );
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
		result = handles.replace( observeParams( resource ), updateResource, "add" );

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

ClientResource.getDestination = function( resource ) {
	return ( resource._private && resource._private.origin ) ? {
		deviceId: null,
		requestUri: resource._private.origin + resource.resourcePath
	} : {
		deviceId: resource.deviceId,
		requestUri: resource.resourcePath
	};
};

module.exports = ClientResource;
