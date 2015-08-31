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
	OicServer = require("./OicServer").OicServer,
	OicClient = require("./OicClient").OicClient,
	_ = require( "underscore" ),
	OicDevice = function( settings ) {
		if ( !this._isOicDevice ) {
			return new OicDevice( settings );
		}

		if ( settings ) {
			this._settings = settings;
		}
		this._client = new OicClient();

		this._server = new OicServer();

	};

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

			//FIXME: See where we can call OCStop?
			var intervalId = setInterval( function() {
				iotivity.OCProcess();
			}, 100 );

			fulfill();
		}, this ) );
	},
} );

module.exports = {
	OicDevice: OicDevice
}