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

function isRepPayload( payload ) {
	return ( payload && payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION &&
				!util.isArray( payload ) && typeof payload.values === "object" );
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
					( ~csdk.OCResourceProperty.OC_SECURE ) ),
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
		return ( 0 |
			( init.discoverable ? csdk.OCResourceProperty.OC_DISCOVERABLE : 0 ) |
			( init.observable ? csdk.OCResourceProperty.OC_OBSERVABLE : 0 ) |
			( init.secure ? csdk.OCResourceProperty.OC_SECURE : 0 ) |
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
		var index, theValue, childValue,
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
			} else {
				result[ index ] = theValue;
			}
		}

		return result;
	},
	makePlatformInfo: function( payload ) {
		var ocPlatformInfo;

		if ( !( payload && payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_PLATFORM &&
				payload.info && typeof payload.info === "object" ) ) {
			return new Error( "platformInfo: invalid payload" );
		}

		ocPlatformInfo = payload.info;

		return _.omitBy( {
			id: ocPlatformInfo.platformID,
			osVersion: ocPlatformInfo.operatingSystemVersion,
			model: ocPlatformInfo.modelNumber,
			manufacturerURL: ocPlatformInfo.manufacturerUrl,
			manufactureDate: ocPlatformInfo.dateOfManufacture ?
				new Date( ocPlatformInfo.dateOfManufacture ) : null,
			supportURL: ocPlatformInfo.supportUrl,
			manufacturerName: ocPlatformInfo.manufacturerName,
			platformVersion: ocPlatformInfo.platformVersion,
			firmwareVersion: ocPlatformInfo.firmwareVersion
		}, _.isNil );
	},
	makeDeviceInfo: function( payload, address ) {
		var addressString;

		if ( !( payload && payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_DEVICE ) ) {
			return _.extend( new Error( "deviceInfo: Invalid payload" ), {
				payload: payload,
				address: address
			} );
		}

		addressString = address.addr;

		if ( address.flags & csdk.OCTransportFlags.OC_IP_USE_V6 ) {
			addressString = "[" + addressString + "]";
		}

		if ( addressString && address.port ) {
			addressString += ":" + address.port;
		}

		return _.omitBy( {
			uuid: payload.sid,
			url: addressString,
			name: payload.deviceName,
			dataModels: payload.dataModelVersions,
			coreSpecVersion: payload.specVersion
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
			secure: object.p.sec,
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
		var index, resource;
		var newResources = [];

		if ( isRepPayload( payload ) ) {
			if ( payload.values.links && payload.values.links.length ) {
				for ( index in payload.values.links ) {
					resource = payloadUtils.resourceFromRepresentation(
						payload.values.links[ index ], payload.values.di );
					if ( resource instanceof Error ) {
						return resource;
					} else {
						newResources.push( resource );
					}
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
				secure: resource.secure,
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
