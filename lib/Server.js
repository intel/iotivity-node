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
var Resource = require( "./Resource" );
var querystring = require( "querystring" );
var payload = require( "./payload" );
var Request = require( "./Request" );
var doAPI = require( "./doAPI" );

var querySeparatorRegex = new RegExp(
	"[" + csdk.OC_QUERY_SEPARATOR.replace( "&", "" ) + "]", "g" );

var Server = function Server() {};
require( "util" ).inherits( Server, require( "events" ).EventEmitter );

function defaultTransform( data ) {
	return _.extend( {}, data );
}

function isValidResource( resource ) {
	return new Promise( function( fulfill, reject ) {
		if ( ( resource instanceof Resource &&
				resource._private &&
				resource._private.handle ) ) {
			fulfill( resource );
		} else {
			reject( _.extend( new Error( "Invalid resource" ), { resource: resource } ) );
		}
	} );
}

function bindStringsToResource( resource, strings, binder, reject ) {
	var index, result, errorMessage;
	var extension = {};

	for ( index = 0; index < strings.length; index++ ) {
		result = csdk[ binder ](
			resource._private.handle, strings[ index ] );
		if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
			errorMessage = "Failed to perform " + binder;
			extension[ binder ] = { value: strings[ index ], result: result };
			result = csdk.OCDeleteResource( resource._private.handle );
			if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
				extension.OCDeleteResource = result;
			}
			reject( _.extend( new Error( errorMessage ), extension ) );
			return false;
		}
	}

	return true;
}

_.extend( Server.prototype, {
	_queryInfo: function( resource, queryString ) {
		var query, existingQueryString;

		queryString = queryString.replace( querySeparatorRegex, "&" );
		query = querystring.parse( queryString );
		existingQueryString = _.findKey( resource._private.observers, function( value, key ) {
			return _.isEqual( querystring.parse( key ), query );
		} );

		return {
			query: query,
			queryString: existingQueryString ? existingQueryString : queryString
		};
	},
	_createEntityHandler: function( resource ) {
		return _.bind( function( flag, request ) {
			var eventData, queryInfo, eventName, observeFlag, queryString;
			var errorExtension = { resource: resource, flag: flag, request: request };

			if ( request.resource && request.resource !== resource._private.handle ) {
				this.emit( "error", _.extend( new Error( "Request received for wrong resource" ),
					errorExtension ) );
				return csdk.OCEntityHandlerResult.OC_EH_ERROR;
			}

			eventName =
				( request.method === csdk.OCMethod.OC_REST_GET ||
					request.method === csdk.OCMethod.OC_REST_OBSERVE ) ? "retrieve" :
				request.method === csdk.OCMethod.OC_REST_POST ?
					( request.payload &&
					request.payload.types &&
					request.payload.uri &&
					request.payload.interfaces ? "create" : "update" ) :
				request.method === csdk.OCMethod.OC_REST_DELETE ? "delete" :
				request.method === csdk.OCMethod.OC_REST_NOMETHOD &&
					flag & csdk.OCEntityHandlerFlag.OC_OBSERVE_FLAG ? "retrieve" :
				_.extend( new Error( "Unknown event" ), errorExtension );

			if ( eventName instanceof Error ) {
				this.emit( "error", eventName );
				return csdk.OCEntityHandlerResult.OC_EH_ERROR;
			}

			eventData = ( eventName === "create" ) ?
				payload.resourceFromRepresentation( request.payload ) :
				( request.payload ? payload.repPayloadToObject( request.payload ) : null );

			if ( eventData instanceof Error ) {
				this.emit( "error", _.extend( eventData, errorExtension ) );
				return csdk.OCEntityHandlerResult.OC_EH_ERROR;
			}

			if ( "query" in request ) {
				queryInfo = this._queryInfo( resource, request.query );
			}

			if ( flag & csdk.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
				if ( request.obsInfo.action === csdk.OCObserveAction.OC_OBSERVE_REGISTER ) {
					observeFlag = true;
					resource._private.observers[ queryInfo.queryString ] =
						( resource._private.observers[ queryInfo.queryString ] ?
							resource._private.observers[ queryInfo.queryString ] : [] )
								.concat( [ request.obsInfo.obsId ] );
				} else if ( request.obsInfo.action ===
						csdk.OCObserveAction.OC_OBSERVE_DEREGISTER ) {
					observeFlag = false;

					// This happens when iotivity decides to stop notifying because of network
					// failure. In such a case we need to use the obsId to reverse-find the query
					// string.
					if ( !queryInfo ) {
						for ( queryString in resource._private.observers ) {
							if ( resource._private.observers[ queryString ]
									.indexOf( request.obsInfo.obsId ) >= 0 ) {
								queryInfo = { queryString: queryString };
								break;
							}
						}
					}

					_.remove( resource._private.observers[ queryInfo.queryString ],
						function( value ) {
							return ( value === request.obsInfo.obsId );
						} );
					if ( resource._private.observers[ queryInfo.queryString ].length === 0 ) {
						delete resource._private.observers[ queryInfo.queryString ];
					}
				}
			}

			this.emit( eventName, Request( _.extend( {
					_type: eventName,
					id: request.requestHandle,
					target: resource
				},
				queryInfo ? { options: queryInfo.query } : {},
				eventData !== null ? { data: eventData } : {},
				observeFlag !== undefined ? { observe: observeFlag } : {} ) ) );

			return csdk.OCEntityHandlerResult.OC_EH_OK;
		}, this );
	},
	register: function( init, transform ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result;
			var resource;
			var handleReceptacle = {};

			if ( !payload.validateResource( init ) ) {
				return reject( _.extend( new Error( "Invalid ResourceInit" ), {
					resourceInit: init
				} ) );
			}
			resource = _.extend( Resource(), init );
			result = csdk.OCCreateResource( handleReceptacle,
				init.resourceTypes[ 0 ],
				init.interfaces[ 0 ],
				init.resourcePath,
				this._createEntityHandler( resource ),
				payload.initToBitmap( init ) );

			if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
				return reject( _.extend( new Error( "register: OCCreateResource() failed" ), {
					result: result
				} ) );
			}

			// observers: {
			//   representation(JSON.stringified object): [ obsId ]
			// }
			resource._private.transform = transform || defaultTransform;
			resource._private.observers = {};
			resource._private.handle = handleReceptacle.handle;
			Resource.setServerResource( resource );

			if ( !( bindStringsToResource( resource, init.resourceTypes.slice( 1 ),
						"OCBindResourceTypeToResource", reject ) &&
					bindStringsToResource( resource, init.interfaces.slice( 1 ),
						"OCBindResourceInterfaceToResource", reject ) ) ) {
				return;
			}

			fulfill( resource );
		}, this ) );
	},
	unregister: function( resource ) {
		return isValidResource( resource )
			.then( function( resource ) {
				return doAPI( "OCDeleteResource", "Failed to delete resource",
					resource._private.handle );
			} );
	},
	notify: function( resource ) {
		return isValidResource( resource )
			.then( function( resource ) {
				return new Promise( function( fulfill, reject ) {
					var query, result, thePayload;
					var errors = [];

					for ( query in resource._private.observers ) {
						thePayload = payload.objectToRepPayload(
							resource._private.transform(
								resource.properties, querystring.parse( query ) ) );
						if ( thePayload instanceof Error ) {
							errors.push( thePayload );
						} else {
							result = csdk.OCNotifyListOfObservers( resource._private.handle,
								resource._private.observers[ query ], thePayload,
								csdk.OCQualityOfService.OC_HIGH_QOS );
							if ( result !== csdk.OCStackResult.OC_STACK_OK ) {
								errors.push( {
									query: query,
									result: result,
									observers: resource._private.observers[ query ].slice( 0 )
								} );
							}
						}
					}

					if ( errors.length > 0 ) {
						reject( errors );
					} else {
						fulfill();
					}
				} );
			} );
	},
	enablePresence: function( timeToLive ) {
		return doAPI( "OCStartPresence", "Failed to enable presence", timeToLive || 0 );
	},
	disablePresence: function() {
		return doAPI( "OCStopPresence", "Failed to disable presence" );
	}
} );

module.exports = new Server();
