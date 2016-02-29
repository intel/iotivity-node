var iotivity = require( "bindings" )( "iotivity" ),
	OicResource = require( "./OicResource" ),
	myUtils = require( "./utils" ),
	util = require( "util" ),
	_ = require( "lodash" ),
	OicRequestEvent = function() {
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

			// Define legacy "onrequest" event handler such that setting it will attach a
			// regular event handler
			myUtils.addLegacyEventHandler( this, "request" );
			return _super.apply( this, arguments );
		};
	} )( devicePrototype._construct ),
	_interestedObservers: [],

	// Extend _startStack()
	_startStack: ( function( _super ) {
		return function( settings, reject ) {
			var result;

			if ( !_super.apply( this, arguments ) ) {
				return false;
			}

			result = iotivity.OCSetDefaultDeviceEntityHandler( this._createEntityHandler() );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend(
					new Error( "server init: OCSetDefaultDeviceEntityHandler() failed" ), {
						result: result
					} ) );
				return false;
			}

			return true;
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

					// As per the C++ Code: The query format is q1=v1&;q1=v2;
					var seperator = "&;";
					var qparams = request.query.split( seperator );
					oicReq.queryOptions = new Array( qparams.length );

					for ( i = 0; request.query !== "" && i < qparams.length; i++ ) {
						var param = qparams[ i ].split( "=" );
						oicReq.queryOptions[ i ] = [];
						oicReq.queryOptions[ i ].key = param[ 0 ];
						oicReq.queryOptions[ i ].value = param[ 1 ];
					}
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
						oicReq.res = myUtils.payloadToObject( request.payload.values );
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
				}

				// FIXME: Check how this should be done.
				oicReq.type = "observe";
			}

			this.dispatchEvent( "request", oicReq );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}, this );
	},

	registerResource: function( init ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var resource;
			var result = 0;
			var flag = 0;
			var handleReceptacle = {};

			if ( init.discoverable ) {
				flag |= iotivity.OCResourceProperty.OC_DISCOVERABLE;
			}

			if ( init.observable ) {
				flag |= iotivity.OCResourceProperty.OC_OBSERVABLE;
			}

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
				reject( _.extend( new Error( "registerResource: OCCreateResource() failed" ), {
					result: result
				} ) );
				return;
			}

			resource._handle = handleReceptacle.handle;
			this._resources[ resource.id.deviceId + ":" + resource.id.path ] = resource;
			fulfill( resource );
		}, this ) );

	},

	unregisterResource: function( resource ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result,
				resourceId = resource.id.deviceId + ":" + resource.id.path;

			if ( !this._resources[ resourceId ] ) {
				reject( new Error( "unregisterResource: resource not found" ) );
				return;
			}

			result = iotivity.OCDeleteResource( this._resources[ resourceId ]._handle );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend(
					new Error( "unregisterResource: OCDeleteResource() failed" ), {
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
