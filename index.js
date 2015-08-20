// Assuming url is of the form host:port, pedantically parse it to produce
// { host: host, port: port }
function getHostInfo( url ) {
	var port,
		returnValue = {};

	if ( url ) {
		url = url.split( ":" );

		// If the host is set, save it
		if ( url[ 0 ] ) {
			returnValue.host = url[ 0 ];

			// We only look for a port if a host is set
			if ( url[ 1 ] ) {
				port = parseInt( url[ 1 ] );
			}

			// Avoid assigning the port if it's undefined or NaN
			if ( port !== undefined && !isNaN( port ) ) {
				returnValue.port = port;
			}
		}
	}

	return returnValue;
}

var iotivity = require( "bindings" )( "iotivity" ),
	_ = require( "underscore" ),
	OicDevice = function( settings ) {
		if ( !this._isOicDevice ) {
			return new OicDevice( settings );
		}

		if ( settings ) {
			this._settings = settings;
		}
	},
	OicResource = function ( init ) {
		if ( !this._isOicResource ) {
			return new OicResource( init );
		}

		if ( init )
			_.extend( this, init );

	},
	OicRequestEvent = function () {
		if ( !this._isOicRequestEvent ) {
			return new OicRequestEvent();
		}
	};

_.extend( OicResource.prototype, {
	_isOicResource: true,
} );

_.extend( OicRequestEvent.prototype, {
	_isOicRequestEvent: true,

	sendResponse: function ( resource ) {

	},
	sendError: function ( error ) {

	}
} );

_.extend( OicDevice.prototype, {
	_isOicDevice: true,
	_isStackStarted: false,

	// OicDeviceSettings: allowed properties:
	// [*] used in this implementation [-] unused
	// * url: string: ( "host:port" )
	// - info: OicDeviceInfo: allowed properties:
	//	 - uuid: string
	//	 - name: string
	//	 - dataModels: Array of strings
	//	 - coreSpecversion: string
	//	 - osVersion: string
	//	 - model: string
	//	 - manufacturerName: string
	//	 - manufacturerUrl: string
	//	 - manufacturerDate: date
	//	 - platformVersion: string
	//	 - firmwareVersion: string
	//	 - supportUrl: string
	// * role: string (OicDeviceRole: "client", "server", or "intermediary")
	// - connectionMode: string (OicConnectionMode: "acked", "non-acked", "default")
	_settings: {

		// By default we launch in both client and server mode for maximum flexibilitys
		role: "intermediary",

		// By defauled we launch in 
		connectionMode: "acked"
	},

	configure: function( settings ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result, hostInfo;

			if ( this._isStackStarted ) {

				result = iotivity.OCStop();
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					reject( _.extend( new Error( "configure: OCStop() failed" ), {
						result: result
					} ) );
					return;
				}
			}

			if ( settings ) {
				this._settings = settings;
			} else {
				settings = this._settings;
			}

			hostInfo = getHostInfo( settings.url );

			result = iotivity.OCInit(
				( ( "host" in hostInfo ) ? hostInfo.host : null ),
				( ( "port" in hostInfo ) ? hostInfo.port : 0 ),
				iotivity.OCMode[
					settings.role === "client" ? "OC_CLIENT" :
					settings.role === "server" ? "OC_SERVER" : "OC_CLIENT_SERVER"
				] );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "configure: OCInit() failed" ), {
					result: result
				} ) );
				return;
			}

			fulfill();
		}, this ) );
	},

	_server: _.extend( {
		onrequest: null,
		listeners: {},

		addEventListener: function ( event, callback ) {
			if ( !( event in this.listeners ) ) {
				this.listeners[ event ] = [];
			}
			// Currently allows duplicate callbacks. Should it?
			this.listeners[ event ].push( callback );
		},

		removeEventListener: function ( event, callback ) {
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

		registerResource: function ( init ) {
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
                  var qparams = request.query.split ( separator );
                  oicReq.queryOptions = new Array (qparams.length);
                  for ( i=0; i< qparams.length; i++ ) {
                    var param = qparams [ i ].split( "=" );
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
                      ._extend ( oicReq.res , obj );

                  } else if ( request.method == iotivity.OCMethod.OC_REST_POST ) {
                      oicReq.type = "update";

                      //FIXME: Check if this is the right way of doing it.
                      var obj = JSON.parse ( request.resJSONPayload );
                      ._extend ( oicReq.res , obj );
                      oicReq.updatedPropertyNames = _.difference ( resource , obj );

                  } else if ( request.method == iotivity.OCMethod.OC_REST_DELETE )
                      oicReq.type = "delete";
                  else if ( request.method == iotivity.OCMethod.OC_REST_OBSERVE )
                      oicReq.type = "observe";
                  //TODO: What about other methods. What is Init method on the api.

              }

              if (flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
                  //TODO:
              }

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

		enablePresence: function ( ttl ) {
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

		disablePresence: function () {
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

	})
} );

module.exports = {
	OicDevice: OicDevice,
	OicResource: OicResource,
	OicRequestEvent: OicRequestEvent
};
