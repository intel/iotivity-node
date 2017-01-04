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
var ServerResource = require( "./ServerResource" );
var querystring = require( "querystring" );
var payload = require( "./payload" );
var Request = require( "./Request" );

var querySeparatorRegex = new RegExp(
	"[" + csdk.OC_QUERY_SEPARATOR.replace( "&", "" ) + "]", "g" );

var Server = function Server() {
	Object.defineProperty( this, "_private", { value: {} } );

	csdk.OCSetDefaultDeviceEntityHandler( this._createEntityHandler() );
};

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
		if ( resource ) {
			existingQueryString = _.findKey( resource._private.observers, function( value, key ) {
				return _.isEqual( querystring.parse( key ), query );
			} );
		}

		return {
			query: query,
			queryString: existingQueryString ? existingQueryString : queryString
		};
	},
	_createEntityHandler: function( resource ) {
		return _.bind( function( flag, request, url ) {
			var eventData, queryInfo, eventName, observeFlag, queryString, theRequest, create;

			// Request coming in for the wrong resource
			if ( request.resource && resource && request.resource !== resource._private.handle ) {
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
				request.method === csdk.OCMethod.OC_REST_PUT && !resource && url ? "create" :
				undefined;

			if ( !eventName ) {
				return csdk.OCEntityHandlerResult.OC_EH_ERROR;
			}

			eventData = ( eventName === "create" ) ?
				payload.resourceFromRepresentation( request.payload ) :
				( request.payload ? payload.repPayloadToObject( request.payload ) : null );

			if ( eventData instanceof Error ) {
				return csdk.OCEntityHandlerResult.OC_EH_ERROR;
			}

			if ( "query" in request ) {
				queryInfo = this._queryInfo( resource, request.query );
			}

			if ( flag & csdk.OCEntityHandlerFlag.OC_OBSERVE_FLAG && resource ) {
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

					if ( queryInfo ) {
						_.remove( resource._private.observers[ queryInfo.queryString ],
							function( value ) {
								return ( value === request.obsInfo.obsId );
							} );
						if ( resource._private.observers[ queryInfo.queryString ].length === 0 ) {
							delete resource._private.observers[ queryInfo.queryString ];
						}
					}
				}
			}

			theRequest = Request( _.extend( {
					type: eventName,
					id: request.requestHandle
				},
				resource ? { target: resource } : {},
				queryInfo ? { options: queryInfo.query } : {},
				eventData !== null ? { data: eventData } : {},
				observeFlag !== undefined ? { observe: observeFlag } : {} ) );

			if ( eventName in { "retrieve": 0, "update": 0, "delete": 0 } && resource ) {
				if ( resource._private[ eventName ] ) {
					resource._private[ eventName ]( theRequest );
				}
			} else if ( eventName === "create" ) {
				create = ( resource ? resource._private.create : this._private.create );
				if ( create ) {
					create( theRequest );
				} else {
					csdk.OCDoResponse( {
						requestHandle: request.requestHandle,
						resourceHandle: resource._private.handle,
						ehResult: csdk.OCEntityHandlerResult.OC_EH_RESOURCE_NOT_FOUND,
						sendVendorSpecificHeaderOption: [],
						resourceUri: resource.resourcePath,
						payload: null
					} );
				}
			}

			return csdk.OCEntityHandlerResult.OC_EH_OK;
		}, this );
	},

	// TODO: assert that this === client in direct create and this === resource in targeted create
	oncreate: function( handler ) {
		this._private.create = _.bind( handler, this );
		return this;
	},
	register: function( init ) {

		return new Promise( _.bind( function( fulfill, reject ) {
			var result;
			var resource;
			var handleReceptacle = {};

			if ( !payload.validateResource( init ) ) {
				return reject( _.extend( new Error( "Invalid ResourceInit" ), {
					resourceInit: init
				} ) );
			}
			resource = _.extend( ServerResource(),
				init.properties ? { properties: init.properties } : {} );
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
			resource._private.observers = {};
			resource._private.handle = handleReceptacle.handle;

			if ( !( bindStringsToResource( resource, init.resourceTypes.slice( 1 ),
						"OCBindResourceTypeToResource", reject ) &&
					bindStringsToResource( resource, init.interfaces.slice( 1 ),
						"OCBindResourceInterfaceToResource", reject ) ) ) {
				return;
			}

			fulfill( resource );
		}, this ) );
	}
} );

module.exports = new Server();
