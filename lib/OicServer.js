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
	OicResource = require( "./OicResource" ),
	myUtils = require( "./utils" ),
	util = require( "util" ),
	querystring = require( "querystring" ),
	_ = require( "lodash" ),
	OicRequestEvent = function OicRequestEvent() {
		if ( !this._isOicRequestEvent ) {
			return new OicRequestEvent();
		}
		myUtils.setPrivate( this, [ "_resourceHandle" ] );
	};

_.extend( OicRequestEvent.prototype, {
	_isOicRequestEvent: true,
	headerOptions: [],

	_constructResponse: function( entityHandlerResult, payloadObject ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result, i,
				headerOptions = [];

			for ( i = 0; i < this.headerOptions.length; i++ ) {
				headerOptions[ i ] = {
					name: this.headerOptions[ i ].name,
					value: this.headerOptions[ i ].value
				};
			}

			result = iotivity.OCDoResponse( {
				requestHandle: this.requestId,
				resourceHandle: this._resourceHandle,
				payload: ( payloadObject ? myUtils.objectToPayload( payloadObject ) : null ),
				sendVendorSpecificHeaderOptions: headerOptions,
				resourceUri: ( iotivity.OCGetResourceUri( this._resourceHandle ) || "" ),
				ehResult: entityHandlerResult
			} );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "OCDoResponse Error" ), {
					result: result
				} ) );
			} else {
				fulfill();
			}
		}, this ) );
	},

	sendResponse: function( resource ) {
		return this._constructResponse( iotivity.OCEntityHandlerResult.OC_EH_OK,
			resource ? resource.properties : null );
	},

	sendError: function( error ) {
		return this._constructResponse( iotivity.OCEntityHandlerResult.OC_EH_ERROR, _.extend( {},

			// Add a message if we can find one
			typeof error === "string" ? { message: error } :
				( util.isError( error ) && error.message ) ? { message: error.message } : {},

			// Add a representation if we can find one
			typeof error !== "string" ? { properties: _.extend( {}, error ) } : {} ) );
	}
} );

module.exports = function( devicePrototype ) {

_.extend( devicePrototype, {
	_construct: ( function( _super ) {
		return function() {

			myUtils.setPrivate( this, [ "_interestedObservers" ] );
			this._interestedObservers = [];

			// Define legacy "on*request" event handlers such that setting one will attach a
			// regular event handler
			myUtils.addLegacyEventHandler( this, "observerequest" );
			myUtils.addLegacyEventHandler( this, "unobserverequest" );
			myUtils.addLegacyEventHandler( this, "retrieverequest" );
			myUtils.addLegacyEventHandler( this, "deleterequest" );
			myUtils.addLegacyEventHandler( this, "createrequest" );
			myUtils.addLegacyEventHandler( this, "updaterequest" );
			return _super.apply( this, arguments );
		};
	} )( devicePrototype._construct ),

	// Extend _startStack()
	_startStack: ( function( _super ) {
		return function() {
			var result;

			_super.apply( this, arguments );

			if ( this._info.device.role === "client" ) {
				return;
			}

			result = iotivity.OCSetDefaultDeviceEntityHandler( this._createEntityHandler() );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				throw _.extend(
					new Error( "server init: OCSetDefaultDeviceEntityHandler() failed" ), {
						result: result
					} );
			}
		};
	} )( devicePrototype._startStack ),

	_createEntityHandler: function( resource ) {
		return _.bind( function( flag, request, requestedUrl ) {

			// Handle the request and raise events accordingly
			var i,
				deviceId = iotivity.OCGetServerInstanceIDString(),
				url = iotivity.OCGetResourceUri( request.resource ) || requestedUrl,
				oicReq = new OicRequestEvent();

			oicReq._resourceHandle = request.resource;
			oicReq.requestId = request.requestHandle;
			oicReq.target = resource || this._resources[ deviceId + ":" + url ] || {
					id: {
						deviceId: deviceId,
						path: url
					}
			};

			if ( flag & iotivity.OCEntityHandlerFlag.OC_REQUEST_FLAG ) {

				if ( request.query ) {
					oicReq.queryOptions = querystring.parse( request.query );
				}

				var len = request.rcvdVendorSpecificHeaderOptions.length;
				oicReq.headerOptions = new Array( len );
				if ( len > 0 ) {
					for ( i = 0; i < len; i++ ) {
						var headerOption = {};

						headerOption.name =
							request.rcvdVendorSpecificHeaderOptions[ i ].optionID;
						headerOption.value =
							request.rcvdVendorSpecificHeaderOptions[ i ].optionData;
						oicReq.headerOptions[ i ] = headerOption;
					}
				}

				if ( request.method === iotivity.OCMethod.OC_REST_GET ) {
					oicReq.type = "retrieve";
				} else if ( request.method === iotivity.OCMethod.OC_REST_POST ) {
					oicReq.type = "create";

					// Make sure we have everything we need for creating a new resource
					if ( !( request.payload &&
						Array.isArray( request.payload.interfaces ) &&
						request.payload.interfaces.length >= 1 &&
						Array.isArray( request.payload.types ) &&
						request.payload.types.length >= 1 ) ) {
						return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
					}

					oicReq.res = {
						id: {
							path: url,
							deviceId: iotivity.OCGetServerInstanceIDString()
						},
						interfaces: request.payload.interfaces,
						resourceTypes: request.payload.types,
						properties: myUtils.payloadToObject( request.payload.values ),

						// FIXME: The client should be able to specify this
						discoverable: true
					};

				} else if ( request.method === iotivity.OCMethod.OC_REST_PUT ) {
					if ( resource ) {
						oicReq.type = "update";

						// FIXME: Check if this is the right way of doing it.
						oicReq.res = request.payload ?
							myUtils.payloadToObject( request.payload.values ) : {};
					}
				} else if ( request.method === iotivity.OCMethod.OC_REST_DELETE ) {
					oicReq.type = "delete";
				} else if ( request.method === iotivity.OCMethod.OC_REST_OBSERVE ) {
					oicReq.type = "observe";
				}

			}

			if ( flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
				if ( request.obsInfo.action ===
						iotivity.OCObserveAction.OC_OBSERVE_REGISTER ) {
					this._interestedObservers.push( request.obsInfo.obsId );
					oicReq.type = "observe";
				} else if ( request.obsInfo.action ===
						iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER ) {
					var index = this._interestedObservers
						.indexOf( request.obsInfo.obsId );

					// FIXME: Should we loop and remove?
					while ( index !== -1 ) {
						this._interestedObservers.splice( index, 1 );
						index = this._interestedObservers
							.indexOf( request.obsInfo.obsId );
					}
					oicReq.type = "unobserve";
				}
			}

			oicReq.type += "request";

			this.dispatchEvent( oicReq.type, oicReq );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}, this );
	},

	register: function( init ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "client" ) {
				return reject( new Error( "Not supported" ) );
			}

			if ( !init.id ) {
				return reject( new Error( "No ID found" ) );
			}

			var resource;
			var result = 0;
			var flag = 0 |
				( init.discoverable ? iotivity.OCResourceProperty.OC_DISCOVERABLE : 0 ) |
				( init.observable ? iotivity.OCResourceProperty.OC_OBSERVABLE : 0 ) |
				( init.slow ? iotivity.OCResourceProperty.OC_SLOW : 0 ) |
				( init.active ? iotivity.OCResourceProperty.OC_ACTIVE : 0 ) |
				( init.secure ? iotivity.OCResourceProperty.OC_SECURE : 0 );
			var handleReceptacle = {};

			if ( !init.id.deviceId ) {
				init.id.deviceId = iotivity.OCGetServerInstanceIDString();
			}

			resource = new OicResource( init );

			result = iotivity.OCCreateResource(
				handleReceptacle,

				// FIXME: API SPEC mentions an array.Vaguely remember that the first type is
				// default from the Oic Spec. Check it up.
				init.resourceTypes[ 0 ],

				// FIXME: API SPEC mentions an array.Vaguely remember that the first type is
				// default from the Oic Spec. Check it up.
				init.interfaces[ 0 ],
				init.id.path,
				this._createEntityHandler( resource ),
				flag );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "register: OCCreateResource() failed" ), {
					result: result
				} ) );
				return;
			}

			resource._handle = handleReceptacle.handle;
			this._resources[ resource.id.deviceId + ":" + resource.id.path ] = resource;
			fulfill( resource );
		}, this ) );

	},

	unregister: function( resource ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "client" ) {
				reject( new Error( "Not supported" ) );
			}

			var result,
				resourceId = resource.id.deviceId + ":" + resource.id.path;

			if ( !this._resources[ resourceId ] ) {
				reject( new Error( "unregister: resource not found" ) );
				return;
			}

			result = iotivity.OCDeleteResource( this._resources[ resourceId ]._handle );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend(
					new Error( "unregister: OCDeleteResource() failed" ), {
						result: result
					} ) );
				return;
			}

			delete this._resources[ resourceId ];
			fulfill();
		}, this ) );
	},

	enablePresence: function( ttl ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "client" ) {
				reject( new Error( "Not supported" ) );
			}

			var result;

			result = iotivity.OCStartPresence( ttl ? ttl : 0 );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "enablePresence: OCStartPresence() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	},

	disablePresence: function() {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "client" ) {
				reject( new Error( "Not supported" ) );
			}

			var result;

			result = iotivity.OCStopPresence();

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "enablePresence: OCStopPresence() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	},

	notify: function( resource ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			if ( this._info.device.role === "client" ) {
				reject( new Error( "Not supported" ) );
			}

			var result;

			if ( this._interestedObservers.length > 0 ) {
				result = iotivity.OCNotifyListOfObservers( resource._handle,
					this._interestedObservers,
					_.extend( myUtils.objectToPayload( resource.properties ), {
						uri: resource.id.path
					} ),
					iotivity.OCQualityOfService.OC_HIGH_QOS );

				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					reject( _.extend(
						new Error( "notify: OCNotifyListOfObservers() failed" ), {
							result: result
						} ) );
					return;
				}
			} else {
				reject( _.extend( new Error( "notify: There are no observers" ),
					{ noObservers: true } ) );
			}

			fulfill();
		}, this ) );
	}
} );

};
