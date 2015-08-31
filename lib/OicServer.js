var iotivity = require( "bindings" )( "iotivity" ),
	OicResource = require( "./OicResource" ).OicResource,
	_ = require( "underscore" ),
	OicServer = function() {
	if ( !this._isOicServer ) {
		return new OicServer();
	}

},
OicRequestEvent = function () {
	if ( !this._isOicRequestEvent ) {
		return new OicRequestEvent();
	}
};


_.extend( OicRequestEvent.prototype, {
	_isOicRequestEvent: true,

	sendResponse: function( resource ) {
		var oicResponse;

		oicResponse.requestHandle = this.requestId;
		oicResponse.resourceHandle = this.resourceHandle;
 		oicResponse.payload = JSON.stringify(resource);
		oicResponse.payloadSize = oicResponse.payload.length;
		oicResponse.numRcvdVendorSpecificHeaderOptions = this.headerOptions.length;
		oicResponse.sendVendorSpecificHeaderOptions = new Array (this.headerOptions.length);
		for (i=0; i<this.headerOptions.length; i++) {
			oicResponse.sendVendorSpecificHeaderOptions [ i ] = [];
			oicResponse.sendVendorSpecificHeaderOptions [ i ].name = this.headerOptions [ i ].name;
			oicResponse.sendVendorSpecificHeaderOptions [ i ].value = this.headerOptions [ i ].value;
		}

		//FIXME: This url should be a FQDN or COAP URL ?
		oicResponse.resourceUri = resource.url;
		oicResponse.ehResult = iotivity.OCEntityHandlerResult.OC_EH_OK;
		var ret = iotivity.OCDoResponse (oicResponse);
		if (ret != iotivity.OCStackResult.OC_STACK_OK)
			console.log ("DoResponse Error");
	},
	sendError: function ( error ) {
		oicResponse.requestHandle = this.requestId;
		oicResponse.resourceHandle = this.resourceHandle;
		oicResponse.ehResult = error.message;		var ret = iotivity.OCDoResponse (oicResponse);
		if (ret != iotivity.OCStackResult.OC_STACK_OK)
			console.log ("DoResponse Error");
	}
} );

_.extend( OicServer.prototype, {
	_isOicServer: true,
	onrequest: null,
	listeners: {},
	_interestedObservers: [],

	addEventListener: function( event, callback ) {
		if ( !( event in this.listeners ) ) {
			this.listeners[ event ] = [];
		}
		// Currently allows duplicate callbacks. Should it?
		this.listeners[ event ].push( callback );
	},

	removeEventListener: function( event, callback ) {
		if ( event in this.listeners ) {
			this.listeners [ event ] = this.listeners [ event ].filter ( function ( ev ) {
				return ev !== callback;
			});
		}
	},

	dispatchEvent: function ( event, request ) {
		if ( typeof this [ "on" + event ] === "function" ) {
			this [ "on" + event ] ( request );
		}
		if ( event in this.listeners ) {
			for ( var i = 0, len = this.listeners [ event ].length; i < len; i++ ) {
				this.listeners [ event ] [ i ].call( this, request );
			}
		}
	},

	registerResource: function( init ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result = 0;
			var flag = 0;
			var handle = {};
			var resource = new OicResource ( init );

			if ( init.discoverable )
				flag |= iotivity.OCResourceProperty.OC_DISCOVERABLE;

			if ( init.observable )
				flag |= iotivity.OCResourceProperty.OC_OBSERVABLE;

			result = iotivity.OCCreateResource (
					handle,

					// FIXME: API SPEC mentions an array.Vaguely remember that the first type is
					// default from the Oic Spec. Check it up.
					init.resourceTypes[ 0 ],

					// FIXME: API SPEC mentions an array.Vaguely remember that the first type is
					// default from the Oic Spec. Check it up.
					init.interfaces[ 0 ],
					init.url,
					function ( flag, request ) {
						// Handle the request and raise events accordingly
						var oicReq = new OicRequestEvent( );

						oicReq.requestId = request.requestHandle;
						oicReq.target = request.resourceHandle;
						oicReq.source = resource;

						if ( flag & iotivity.OCEntityHandlerFlag.OC_REQUEST_FLAG ) {

							// Asper the C++ Code: The query format is q1=v1&;q1=v2;
							var seperator = "&;";
							var qparams = request.query.split ( seperator );
							oicReq.queryOptions = new Array (qparams.length);

							for ( i=0; request.query != "" && i< qparams.length; i++ ) {
								var param = qparams [ i ].split( "=" );
								oicReq.queryOptions [ i ] = [];
								oicReq.queryOptions [ i ].key = param [ 0 ];
								oicReq.queryOptions [ i ].value = param [ 1 ];
							}

							oicReq.headerOptions = new Array ( request.numRcvdVendorSpecificHeaderOptions );
							if ( request.numRcvdVendorSpecificHeaderOptions != 0 ) {
								  for ( i=0; i< request.numRcvdVendorSpecificHeaderOptions; i++ ) {
									var headerOption = new Object ();
									headerOption.name = request.rcvdVendorSpecificHeaderOptions [ i ].optionID;
									headerOption.value = request.rcvdVendorSpecificHeaderOptions [ i ].optionData;
									oicReq.headerOptions [ i ] = headerOption;
								}
							}

							if ( request.method == iotivity.OCMethod.OC_REST_GET )
								oicReq.type = "retrieve";
							else if ( request.method == iotivity.OCMethod.OC_REST_PUT ) {
								oicReq.type = "create";

								var obj = JSON.parse ( request.resJSONPayload );
								_.extend ( oicReq.res , obj );

							} else if ( request.method == iotivity.OCMethod.OC_REST_POST ) {
								oicReq.type = "update";

								//FIXME: Check if this is the right way of doing it.
								var obj = JSON.parse ( request.resJSONPayload );
								_.extend ( oicReq.res , obj );
								oicReq.updatedPropertyNames = _.difference ( resource , obj );
							} else if ( request.method == iotivity.OCMethod.OC_REST_DELETE )
								oicReq.type = "delete";
							else if ( request.method == iotivity.OCMethod.OC_REST_OBSERVE )
								oicReq.type = "observe";

						}

						if ( flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
							if ( request.obsInfo.action == iotivity.OCObserveAction.OC_OBSERVE_REGISTER ) {
								this._interestedObservers.push( request.obsInfo.obsId );
							} else if ( request.obsInfo.action == iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER ) {
								var index = _interestedObservers.indexOf( request.obsInfo.obsId );
								//FIXME: Should we loop and remove?
								while ( index != -1 ) {
									this._interestedObservers.splice ( index, 1 );
									index = this._interestedObservers.indexOf( request.obsInfo.obsId );
								}
							}

							//FIXME: Check how this should be done.
							oicReq.type = "observe";
						}

						console.log ( oicReq );
						dispatchEvent ( "request", oicReq );
					},
					flag );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "registerResource: OCCreateResource() failed" ), {
					result: result
				} ) );
				return;
			}

			// FIXME: What whould be the id? Something unique or the handle?
			resource.id = handle;

			fulfill( resource );
		}, this ) );

	},

	unregisterResource: function( resourceId ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result;

			result = iotivity.OCDeleteResource ( resourceId );

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "unregisterResource: OCDeleteResource() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	},

	enablePresence: function( ttl ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result;

			result = iotivity.OCStartPresence ( ttl ? ttl : 0 );

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

			result = iotivity.OCStopPresence ();

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "enablePresence: OCStopPresence() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	},

	notify: function( resourceId, method, updatedPropertyNames ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result;

			//FIXME: Find how we define QoS
			result = iotivity.OCNotifyListOfObservers ( resourceId,
				_interestedObservers,
				_interestedObservers.length,
				JSON.stringify(updatedPropertyNames),
				iotivity.OCQualityOfService.OC_NA_QOS
				);

			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "notify: OCNotifyListOfObservers() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	}
})


module.exports = {
	OicServer: OicServer,
	OicRequestEvent: OicRequestEvent
}