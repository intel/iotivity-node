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

	};

_.extend( OicResource.prototype, {
	_isOicResource: true,
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
			if ( !( event in listeners ) ) {
				listeners[ event ] = [];
			}
			// Currently allows duplicate callbacks. Should it?
			listeners[ event ].push( callback );
		},

		removeEventListener: function ( event, callback ) {
			if ( event in listeners ) {
				listeners [ event ] = listeners [ event ].filter ( function ( ev ) {
					return ev !== callback;
				});
			}
		},

		dispatchEvent: function ( event, request ) {
			if ( typeof self [ "on" + event ] === "function" ) {
				self [ "on" + event ] ( request );
			}
			if ( event in listeners ) {
				for ( var i = 0, len = listeners [ event ].length; i < len; i++ ) {
					listeners [ event ] [ i ].call( self, request );
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

						// FIXME: API SPEC mentions an array.Vagule remember that the first type is
						// default from the Oic Spec. Check it up.
						init.resourceTypes[ 0 ],

						// FIXME: API SPEC mentions an array.Vagule remember that the first type is
						// default from the Oic Spec. Check it up.
						init.interfaces[ 0 ],
						init.url,
						function ( flag, request ) {

							// Handle the request and raise events accordingly
						},
						flag );

				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					reject( _.extend( new Error( "enablePresence: OCStartPresence() failed" ), {
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
	OicResource: OicResource
};
