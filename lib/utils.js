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
	util = require( "util" );

function objectToPayload( theObject, visitedObjects ) {
	var index, theValue,
		payload = { type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION, values: {} };

	if ( visitedObjects === undefined ) {
		visitedObjects = {};
	}

	for ( index in theObject ) {
		theValue = theObject[ index ];
		if ( typeof theValue === "object" && theValue !== null ) {
			if ( visitedObjects[ theValue ] ) {
				throw new Error( "objectToPayload: Cannot handle circular object references" );
			}
			visitedObjects[ theValue ] = true;
			if ( !util.isArray( theValue ) ) {
				payload.values[ index ] = objectToPayload( theValue, visitedObjects );
				continue;
			}
		}
		payload.values[ index ] = theValue;
	}

	return payload;
}

function payloadToObject( values ) {
	var index, theValue,
		result = {};

	for ( index in values ) {
		theValue = values[ index ];
		if ( typeof theValue === "object" && !util.isArray( theValue ) && theValue !== null ) {
			result[ index ] = payloadToObject( theValue.values );
		} else {
			result[ index ] = theValue;
		}
	}

	return result;
}

module.exports = {
	setPrivate: function( object, props ) {
		var index;

		for ( index in props ) {
			Object.defineProperty( object, props[ index ], {
				writable: true,
				configurable: true
			} );
		}
	},

	// Alias "on<event>" properties to (add|remove)Listener
	addLegacyEventHandler: function( object, name ) {
		var handler;
		Object.defineProperty( object, "on" + name, {
			configurable: true,
			enumerable: true,
			get: function() {
				return handler;
			},
			set: function( newValue ) {
				if ( handler ) {
					object.removeListener( name, handler );
					handler = undefined;
				}
				if ( newValue ) {
					object.addListener( name, newValue );
					handler = newValue;
				}
			}
		} );
	},
	makeEventEmitter: function( theObject ) {
		util.inherits( theObject, require( "events" ).EventEmitter );
		_.extend( theObject.prototype, {
			addEventListener: theObject.prototype.addListener,
			removeEventListener: theObject.prototype.removeListener,
			dispatchEvent: theObject.prototype.emit
		} );
		return theObject;
	},
	objectToPayload: objectToPayload,
	payloadToObject: payloadToObject
};
