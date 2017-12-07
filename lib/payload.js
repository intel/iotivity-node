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
	omitBy: require( "lodash.omitby" ),
	isNil: require( "lodash.isnil" )
};
var util = require( "util" );
var csdk = require( "./csdk" );

var uuidRegex = /[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}/;

function isRepPayload( payload ) {
	return ( payload && payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
				!util.isArray( payload ) );
}

function isNonEmptyStringArray( theArray ) {
	var index;

	if ( Array.isArray( theArray ) && theArray.length > 0 ) {
		for ( index in theArray ) {
			if ( typeof theArray[ index ] !== "string" ) {
				return false;
			}
		}
		return true;
	}

	return false;
}

var payloadUtils = {
	initToCreateRequestPayload: function( init ) {
		var payload = payloadUtils.objectToRepPayload( {
			href: init.resourcePath,
			p: {
				bm: ( payloadUtils.initToBitmap( init ) &
					( ~( csdk.OCResourceProperty.OC_SECURE |
						csdk.OCResourceProperty.OC_NONSECURE ) ) ),
				sec: !!( init.secure )
			}
		} );

		return payload instanceof Error ? payload : _.extend( payload, {
			types: init.resourceTypes,
			interfaces: init.interfaces
		} );
	},
	validateResource: function( init ) {
		return ( init.resourcePath && typeof init.resourcePath === "string" &&
			isNonEmptyStringArray( init.resourceTypes ) &&
			isNonEmptyStringArray( init.interfaces ) );
	},
	initToBitmap: function( init ) {
		return ( csdk.OCResourceProperty.OC_SECURE |
			( init.discoverable ? csdk.OCResourceProperty.OC_DISCOVERABLE : 0 ) |
			( init.observable ? csdk.OCResourceProperty.OC_OBSERVABLE : 0 ) |
			( init.secure ? 0 : csdk.OCResourceProperty.OC_NONSECURE ) |
			( init.slow ? csdk.OCResourceProperty.OC_SLOW : 0 ) |
			( init.active ? csdk.OCResourceProperty.OC_ACTIVE : 0 ) );
	},
	objectToRepPayload: function( theObject, visitedObjects ) {
		var index, theValue, childPayload,
			payload = { type: csdk.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION, values: {} };

		if ( visitedObjects === undefined ) {
			visitedObjects = {};
		}

		for ( index in theObject ) {
			theValue = theObject[ index ];
			if ( typeof theValue === "object" && theValue !== null ) {
				if ( visitedObjects[ theValue ] ) {
					return new Error( "objectToRepPayload: Circular object reference" );
				}
				visitedObjects[ theValue ] = true;
				if ( !util.isArray( theValue ) ) {
					childPayload = payloadUtils.objectToRepPayload( theValue, visitedObjects );
					if ( childPayload instanceof Error ) {
						return childPayload;
					}
					payload.values[ index ] = childPayload;
					continue;
				}
			}
			payload.values[ index ] = theValue;
		}

		return payload;
	},
	repPayloadToObject: function( payload ) {
		var index, childIndex, theValue, childValue,
			result = {};

		if ( !isRepPayload( payload ) ) {
			return _.extend( new Error( "repPayloadToObject: Invalid representation payload" ), {
				payload: payload
			} );
		}

		for ( index in payload.values ) {
			theValue = payload.values[ index ];
			if ( isRepPayload( theValue ) ) {
				childValue = payloadUtils.repPayloadToObject( theValue );
				if ( childValue instanceof Error ) {
					return childValue;
				}
				result[ index ] = childValue;
			} else if ( Array.isArray( theValue ) ) {
				result[ index ] = [];
				for ( childIndex in theValue ) {
					result[ index ]
						.push( typeof theValue[ childIndex ] === "object" ?
							payloadUtils.repPayloadToObject( theValue[ childIndex ] ) :
							theValue[ childIndex ] );
					if ( result[ index ][ childIndex ] instanceof Error ) {
						return result[ index ][ childIndex ];
					}
				}
			} else {
				result[ index ] = theValue;
			}
		}

		return result;
	},
	makePlatformInfo: function( payload ) {
		payload = payloadUtils.repPayloadToObject( payload );
		if ( payload instanceof Error ) {
			return payload;
		}

		return _.omitBy( {

			// Where is payload.{mnhw,st,vid} ?
			id: payload.pi,
			osVersion: payload.mnos,
			model: payload.mnmo,
			manufacturerURL: payload.mnml,
			manufactureDate: payload.mndt ? new Date( payload.mndt ) : null,
			supportURL: payload.mnsl,
			manufacturerName: payload.mnmn,
			platformVersion: payload.mnpv,
			firmwareVersion: payload.mnfv
		}, _.isNil );
	},
	makeDeviceInfo: function( payload, address ) {
		var addressString;

		payload = payloadUtils.repPayloadToObject( payload );
		if ( payload instanceof Error ) {
			return _.extend( payload, { address: address } );
		}

		addressString = address.addr;

		if ( address.flags & csdk.OCTransportFlags.OC_IP_USE_V6 ) {
			addressString = "[" + addressString + "]";
		}

		if ( addressString && address.port ) {
			addressString += ":" + address.port;
		}

		return _.omitBy( {
			uuid: payload.di,
			url: addressString,
			name: payload.n,
			dataModels: payload.dmv,
			coreSpecVersion: payload.icv
		}, _.isNil );
	},
	resourceFromRepresentation: function( payload, deviceId ) {
		var resource;
		var object = payloadUtils.repPayloadToObject( payload );

		if ( object instanceof Error ) {
			return _.extend( object, { deviceId: deviceId } );
		}

		resource = _.extend( {
			deviceId: deviceId,
			resourcePath: object.href || payload.uri,
			resourceTypes: object.rt || payload.types,
			interfaces: object.if || payload.interfaces
		}, {
			secure: ( object.eps || [] ).filter( function( item ) {
				return item.ep.substr( 0, 5 ) !== "coaps";
			} ).length === 0,
			discoverable: !!( object.p.bm & csdk.OCResourceProperty.OC_DISCOVERABLE ),
			observable: !!( object.p.bm & csdk.OCResourceProperty.OC_OBSERVABLE ),
			slow: !!( object.p.bm & csdk.OCResourceProperty.OC_SLOW ),
			active: !!( object.p.bm & csdk.OCResourceProperty.OC_ACTIVE )
		} );

		return payloadUtils.validateResource( resource ) ? resource :
			_.extend( new Error( "Invalid resource representation from device" ), {
				deviceId: deviceId,
				payload: payload
			} );
	},
	processGetOicRes: function( payload ) {
		var resource;
		var newResources = [];

		if ( isRepPayload( payload ) ) {
			for ( ; payload; payload = payload.next ) {
				resource = payloadUtils.resourceFromRepresentation( payload,
					( payload.values.anchor.match( uuidRegex ) || [] )[ 0 ] );
				if ( resource instanceof Error ) {
					return resource;
				} else {
					newResources.push( resource );
				}
			}
		} else {
			return _.extend( new Error( "processGetOicRes: unexpected payload" ), {
				payload: payload
			} );
		}

		return newResources;
	},
	processDiscoveryPayload: function( payload ) {
		var index, resource;
		var newResources = [];

		if ( !( payload && payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_DISCOVERY &&
				payload.resources && payload.resources.length ) ) {
			return _.extend( new Error( "processDiscoveryPayload: invalid payload" ), {
				payload: payload
			} );
		}

		for ( index in payload.resources ) {
			resource = payload.resources[ index ];
			if ( !( resource.uri && resource.types && resource.types.length > 0 &&
					resource.interfaces && resource.interfaces.length > 0 &&
					"bitmap" in resource && "secure" in resource ) ) {
				return _.extend(
					new Error( "processDiscoveryPayload: invalid resource in payload" ), {
						payload: payload
					} );
			}
			newResources.push( {
				deviceId: payload.sid,
				resourcePath: resource.uri,
				resourceTypes: resource.types,
				interfaces: resource.interfaces,
				secure: resource.eps.filter( function( item ) {
					return item.tps !== "coaps";
				} ).length === 0,
				discoverable: !!( resource.bitmap &
					csdk.OCResourceProperty.OC_DISCOVERABLE ),
				observable: !!( resource.bitmap &
					csdk.OCResourceProperty.OC_OBSERVABLE ),
				slow: !!( resource.bitmap &
					csdk.OCResourceProperty.OC_SLOW ),
				active: !!( resource.bitmap &
					csdk.OCResourceProperty.OC_ACTIVE )
			} );
		}

		return newResources;
	}
};

module.exports = payloadUtils;
