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

var client;

var _ = {
	extend: require( "lodash.assignin" ),
	isEqual: require( "lodash.isequal" )
};
var util = require( "util" );
var csdk = require( "./csdk" );
var handles = require( "./ClientHandles" );
var payload = require( "./payload" );
var ClientResource = require( "./ClientResource" );
var resolver = require( "./Resolver" );
var querystring = require( "querystring" );
var listenerCount = require( "./listenerCount" );

var Client = function Client() {};
require( "util" ).inherits( Client, require( "events" ).EventEmitter );

function makeDevice( thePayload, address ) {
	var result = payload.makeDeviceInfo( thePayload, address );

	// Side-effect: Update resolver using the device found
	if ( !( result instanceof Error ) ) {
		resolver.add( result.uuid, address );
	}

	return result;
}

// Prefer an endpoint to a resource address if an endpoint is given.
function establishDestination( resourceId, endpoint ) {
	return endpoint ? {
		deviceId: null,
		requestUri: endpoint + resourceId.requestUri
	} : resourceId;
}

function makeResources( resources, address, resolve, filterPath, emit ) {
	var resource, index;
	var filteredResources = [];

	if ( resources instanceof Error ) {
		return resources;
	}

	// Side-effect: Update the resolver with the address of the device from which
	// the new resources have arrived
	if ( resources.length && resources[ 0 ].deviceId ) {
		resolver.add( resources[ 0 ].deviceId, address );
	}

	for ( index in resources ) {
		if ( !filterPath || filterPath === resources[ index ].resourcePath ) {
			resource = ClientResource( resources[ index ] );
			filteredResources.push( resource );
			if ( emit ) {
				client.emit( "resourcefound", resource );
			}
		}
	}

	return resolve ?
		( resource ? resource : new Error( "Resource not found" ) ) :
		( emit ? undefined : filteredResources );
}

function plainFinder( prefix, eventName, listener, uri, query, itemMaker ) {
	return new Promise( function( fulfill, reject ) {
		var anError;

		if ( listener ) {
			client.on( eventName, listener );
		}

		anError = handles.replace( _.extend( {
			method: "OC_REST_DISCOVER",
			requestUri: uri
		}, query ? { query: query } : {} ), function( response ) {
			var index, newItem;

			if ( !( response && response.result === csdk.OCStackResult.OC_STACK_OK ) ) {
				newItem = _.extend( new Error( prefix + ": Unexpected response" ), {
						response: response
					} );
			} else {
				newItem = itemMaker( response.payload, response.addr );
			}

			if ( newItem instanceof Error ) {
				client.emit( "error", newItem );
			} else if ( util.isArray( newItem ) ) {
				for ( index in newItem ) {
					client.emit( eventName, newItem[ index ] );
				}
			} else if ( newItem ) {
				client.emit( eventName, newItem );
			}
		} );

		if ( anError ) {
			reject( anError );
		} else {
			fulfill();
		}
	} );
}

// options: {
//   deviceId:     string -> uuid of device to be passed to resolver
//                 object -> OCDevAddr to use as destination (undocumented)
//                 absent -> destination is null
//   method:       string -> "OC_REST_*"
//   requestUri:   string
//   query:        object -> query options to be stringified
//   payload:      object
//   prefix:       string -> to use in error messages
//   expected:     number -> csdk.OCStackResult.*
//   createAnswer: function( response ) {} -> creates the answer with which to resolve the promise.
//                                            If the answer is an Error, reject instead of fulfill.
// }
function oneShotRequest( options ) {
	return new Promise( function( fulfill, reject ) {
		var result;
		var handleReceptacle = {};
		var destination = null;

		if ( options.deviceId ) {
			destination = typeof options.deviceId === "object" ?
				options.deviceId : resolver.get( options.deviceId );
			if ( destination instanceof Error ) {
				reject( destination );
				return;
			}
		}

		result = csdk.OCDoResource( handleReceptacle,
			csdk.OCMethod[ options.method ],
			options.requestUri +
				( options.query ? ( "?" + querystring.stringify( options.query ) ) : "" ),
			destination,
			options.payload || null,
			csdk.OCConnectivityType.CT_DEFAULT,
			csdk.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {

				setImmediate( function() {
					var answer;

					if ( response && ( response.result === csdk.OCStackResult.OC_STACK_OK ||
							response.result === csdk.OCStackResult[ options.expected ] ) ) {
						if ( options.createAnswer ) {
							answer = options.createAnswer( response );
						}
					} else {
						answer = _.extend( new Error(
							( response &&
							response.result === csdk.OCStackResult.OC_STACK_UNAUTHORIZED_REQ ) ?
								"Access denied" : options.prefix + ": unexpected response" ),
							{ response: response } );
					}

					( ( answer instanceof Error ) ? reject : fulfill )
						.apply( this, answer ? [ answer ] : [] );
				} );

				return csdk.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			null );

		if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
			reject( _.extend( new Error( options.prefix + ": request failed" ), {
				result: result
			} ) );
		}
	} );
}

var isDuplicateSequence = ( function() {
	var sequenceList = [];
	var sequenceHash = {};
	var maxSequences = 5;

	return function( sequence ) {
		if ( typeof sequence === "number" ) {
			sequence = ( "" + sequence );
			if ( sequence in sequenceHash ) {
				return true;
			}
			sequenceHash[ sequence ] = 0;
			sequenceList.push( sequence );
			if ( sequenceList.length > maxSequences ) {
				sequence = sequenceList.shift();
				delete sequenceHash[ sequence ];
			}
		}
	};
} )();

function globalPresenceListener( response ) {
	if ( !( response && response.payload &&
			response.payload.type === csdk.OCPayloadType.PAYLOAD_TYPE_PRESENCE ) ) {
		client.emit( "error", _.extend( new Error( "Invalid presence response" ), {
			response: response
		} ) );
	} else if ( !isDuplicateSequence( response.sequenceNumber ) ) {
		if ( response.payload.trigger === csdk.OCPresenceTrigger.OC_PRESENCE_TRIGGER_DELETE ) {
			client.getDeviceInfo( response.addr )
				.then( function( device ) {
					return { device: device, requestSuccessful: true };
				} )

				// It's entirely expected that getDeviceInfo() will fail, since the device has
				// announced that it's going away. Still, we try to grab the info, but if it fails,
				// we try a reverse lookup to see if we can get the deviceId. If the reverse lookup
				// succeeds, we construct a partial Device with just the uuid field for the
				// 'devicelost' event.
				.catch( function( error ) {
					var device = { uuid: resolver.get( response.addr ) };

					if ( device.uuid instanceof Error ) {
						client.emit( "error", _.extend( new Error(
							"Unable to establish device ID for device which was lost" ), {
								response: response,
								requestError: error,
								resolveError: device.uuid
							} ) );
					} else {
						return { device: device, requestSuccessful: false };
					}
				} )
				.then( function( info ) {
					if ( info ) {
						if ( info.requestSuccessful ) {

							// This is a multicast address, but we must use it in a unicast call,
							// so let's remove the multicast flag.
							response.addr.flags &= ~csdk.OCTransportFlags.OC_MULTICAST;
							return oneShotRequest( {
								deviceId: response.addr,
								method: "OC_REST_GET",
								requestUri: csdk.OC_MULTICAST_DISCOVERY_URI,
								prefix: "devicelost",
								createAnswer: function( getResponse ) {
									var resources =
										payload.processGetOicRes( getResponse.payload );

									return ( resources instanceof Error ) ? resources :
										_.extend( info, { resources: resources } );
								}
							} );
						} else {
							resolver.remove( info.device.uuid );
							client.emit( "devicelost", info.device );
						}
					}
				} )
				.then(
					function( info ) {
						if ( info ) {
							client.emit( "_resourceslost", info );
						}
					},
					function() {} );
		} else if ( response.payload.trigger ===
				csdk.OCPresenceTrigger.OC_PRESENCE_TRIGGER_CREATE ) {

			client.getDeviceInfo( response.addr )
				.then( function( device ) {
					client.emit( "devicefound", device );
					return false;
				},
				function() {
					return true;
				} )
				.then( function( needDeviceFound ) {
					return Promise.all( [ needDeviceFound, client.findResources( {
						deviceId: response.addr,
						_resolve: needDeviceFound
					} ) ] );
				} )
				.then( function( result ) {
					if ( result[ 0 ] ) {
						client.emit( "devicefound", { uuid: result[ 1 ] } );
					}
				} )
				.catch( function( error ) {
					client.emit( "error", _.extend( error, {
						presenceEvent: response
					} ) );
				} );
		}
	}
}

client = _.extend( new Client(), {
	getDeviceInfo: function( deviceId ) {
		return oneShotRequest( {
			prefix: "getDeviceInfo",
			method: "OC_REST_GET",
			requestUri: csdk.OC_RSRVD_DEVICE_URI,
			deviceId: deviceId,
			createAnswer: function( response ) {
				return makeDevice( response.payload, response.addr );
			}
		} );
	},
	getPlatformInfo: function( deviceId ) {
		return oneShotRequest( {
			prefix: "getPlatformInfo",
			method: "OC_REST_GET",
			requestUri: csdk.OC_RSRVD_PLATFORM_URI,
			deviceId: deviceId,
			createAnswer: function( response ) {
				return payload.makePlatformInfo( response.payload );
			}
		} );
	},
	create: function( resourceInit, target, endpoint ) {
		var options = _.extend( establishDestination( target ? {
			deviceId: target.deviceId,
			requestUri: target.resourcePath,
			method: "OC_REST_POST"
		} : {
			deviceId: resourceInit.deviceId,
			requestUri: resourceInit.resourcePath,
			method: "OC_REST_PUT"
		}, endpoint ), { payload: payload.initToCreateRequestPayload( resourceInit ) } );

		return options.payload instanceof Error ? Promise.reject( options.payload ) :
			oneShotRequest( _.extend( options, {
				prefix: "create",
				expected: "OC_STACK_RESOURCE_CREATED",
				createAnswer: function( response ) {
					var init =
						payload.resourceFromRepresentation( response.payload, options.deviceId );
					if ( init instanceof Error ) {
						return init;
					}
					return ClientResource( init );
				}
			} ) );
	},
	retrieve: function( resourceId, query, listener, endpoint ) {

		// Argument juggling
		// If @query is a function, then the query is not specified, but a listener is, so we
		// shift the arguments around and initialize @query
		if ( arguments.length === 2 && typeof query === "function" ) {
			listener = query;
			query = undefined;
		}

		return Promise.resolve( ( resourceId instanceof ClientResource ) ?
			( _.isEqual( resourceId._private.query, query ) ?
					resourceId :
					ClientResource( resourceId, true ) ) :
				ClientResource( resourceId ) )
			.then( function( resource ) {
				var get = function() {
					return oneShotRequest( _.extend( {}, {
						prefix: "retrieve",
						method: "OC_REST_GET",
					}, establishDestination( {
						deviceId: resource.deviceId,
						requestUri: resource.resourcePath,
					}, endpoint ), {
						query: query,
						createAnswer: function( response ) {
							var properties = response.payload ?
								payload.repPayloadToObject( response.payload ) : {};
							if ( properties instanceof Error ) {
								return properties;
							} else {
								_.extend( resource.properties, properties );
								return resource;
							}
						}
					} ) );
				};

				// Make sure the requested query (if any) is associated with this resource
				if ( !( "query" in resource._private ) && query ) {
					resource._private.query = query;
				}

				return listener ?
					new Promise( function( fulfill, reject ) {
						var result = ClientResource.observe( resource, fulfill, reject );

						if ( !( result instanceof Error ) ) {
							resource.on( "update", listener );
						}

						// When ClientResource.observe() returns true it means it is unable to
						// resolve the promise, which in turn means that we must perform a get().
						// So, let's resolve this promise with a special value (true), so that the
						// chained promise can call get().
						if ( result ) {
							fulfill( true );
						}
					} ).then( function( result ) {
						return ( result === true ? get() : result );
					} ) :
					get();
			} ).catch( function( error ) {
				return Promise.reject( error );
			} );
	},
	update: function( resource, endpoint ) {
		var properties = resource.properties ?
			payload.objectToRepPayload( resource.properties ) : null;
		if ( properties instanceof Error ) {
			return Promise.reject( properties );
		}

		return oneShotRequest( _.extend(
			( resource._private ? { query: resource._private.query } : {} ), {
			prefix: "update",
			method: "OC_REST_POST",
		}, establishDestination( {
			requestUri: resource.resourcePath,
			deviceId: resource.deviceId,
		}, endpoint ), {
			payload: properties,
			expected: "OC_STACK_RESOURCE_CHANGED",
			createAnswer: function( response ) {
				var newProperties;
				if ( response.payload ) {
					newProperties = payload.repPayloadToObject( response.payload );
					if ( newProperties instanceof Error ) {
						return newProperties;
					}
					_.extend( resource.properties, newProperties );
				}
				return resource;
			}
		} ) );
	},
	delete: function( resourceId, endpoint ) {
		return oneShotRequest( _.extend( {}, {
			prefix: "delete",
			method: "OC_REST_DELETE",
			expected: "OC_STACK_RESOURCE_DELETED",
		}, establishDestination( {
			requestUri: resourceId.resourcePath,
			deviceId: resourceId.deviceId
		}, endpoint ) ) );
	},

	findDevices: function( listener ) {
		return plainFinder( "findDevices", "devicefound", listener, csdk.OC_RSRVD_DEVICE_URI,
			null, makeDevice );
	},
	findPlatforms: function( listener ) {
		return plainFinder( "findPlatforms", "platformfound", listener, csdk.OC_RSRVD_PLATFORM_URI,
			null, payload.makePlatformInfo );
	},
	findResources: function( options, listener ) {
		var query;

		// If only one argument of type function is passed, it's the listener
		if ( arguments.length === 1 && typeof options === "function" ) {
			listener = options;
			options = {};
		}
		options = options || {};

		query = options.resourceType ? { rt: options.resourceType } : null;

		if ( options.deviceId ) {
			if ( listener ) {
				client.on( "resourcefound", listener );
			}
			return oneShotRequest( _.extend( {
				prefix: "findResources",
				method: "OC_REST_GET",
				requestUri: csdk.OC_MULTICAST_DISCOVERY_URI,
				deviceId: options.deviceId,
				query: query,
				createAnswer: function( response ) {
					return makeResources( payload.processGetOicRes( response.payload ),
						response.addr, options._resolve, options.resourcePath, true );
				}
			}, query ? { query: query } : {} ) );
		} else {
			return plainFinder( "findResources", "resourcefound", listener,
				csdk.OC_MULTICAST_DISCOVERY_URI, query, function( discoveryPayload, address ) {
					return makeResources( payload.processDiscoveryPayload( discoveryPayload ),
						address, false, options.resourcePath );
				} );
		}
	}
} );

function handlePresenceListener() {
	var callingArguments = arguments;
	return function( eventName, listener ) {
		var result;

		if ( ( eventName === "devicefound" || eventName === "devicelost" ) &&
				listenerCount( client, "devicefound" ) === 0 &&
				listenerCount( client, "devicelost" ) === 0 ) {
			result = handles.replace.apply( handles, [ {
				method: "OC_REST_DISCOVER",
				requestUri: csdk.OC_RSRVD_PRESENCE_URI
			}, globalPresenceListener ]
				.concat( Array.prototype.slice.call( callingArguments ) ) );
			if ( result instanceof Error ) {
				client.emit( "error", _.extend( result, {
					eventName: eventName,
					listener: listener
				} ) );
			}
		}
	};
}

client
	.on( "newListener", handlePresenceListener() )
	.on( "removeListener", handlePresenceListener( "remove" ) );

ClientResource.client = client;

module.exports = client;
