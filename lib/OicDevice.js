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

		this._client = require( "./OicClient" )( this );
		this._server = require( "./OicServer" )( this );
	};

_.extend( OicDevice.prototype, {
	_isOicDevice: true,
	_OCProcessIntervalId: null,

	// I OicDeviceSettings: allowed properties:
	// I [*] used in this implementation [-] unused
	// I * url: string: ( "host:port" )
	// I - info: OicDeviceInfo: allowed properties:
	// I - uuid: string
	// I - name: string
	// I - dataModels: Array of strings
	// I - coreSpecversion: string
	// I - osVersion: string
	// I - model: string
	// I - manufacturerName: string
	// I - manufacturerUrl: string
	// I - manufacturerDate: date
	// I - platformVersion: string
	// I - firmwareVersion: string
	// I - supportUrl: string
	// I * role: string (OicDeviceRole: "client", "server", or "intermediary")
	// I - connectionMode: string (OicConnectionMode: "acked", "non-acked", "default")
	_settings: {

		// By default we launch in both client and server mode for maximum flexibility
		role: "intermediary",

		// By default we launch in acked mode
		connectionMode: "acked"
	},

	configure: function( settings ) {
		return new Promise( _.bind( function( fulfill, reject ) {
			var result, hostInfo;

			if ( this._OCProcessIntervalId ) {

				result = iotivity.OCStop();
				if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
					reject( _.extend( new Error( "configure: OCStop() failed" ), {
						result: result
					} ) );
					return;
				} else {
					clearInterval( this._OCProcessIntervalId );
					this._OCProcessIntervalId = null;
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
				iotivity.OCMode[ settings.role === "client" ? "OC_CLIENT" :
					settings.role === "server" ? "OC_SERVER" : "OC_CLIENT_SERVER" ] );
			if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
				reject( _.extend( new Error( "configure: OCInit() failed" ), {
					result: result
				} ) );
				return;
			} else {
				this._OCProcessIntervalId = setInterval( iotivity.OCProcess, 100 );
			}

			// Set device and platform info, if present. iotivity appends something similar to
			// info.dataModels and info.coreSpecVersion to the device info it sends out.
			if ( settings.info ) {

				// Device name must be present if we're to call OCSetDeviceInfo
				if ( settings.info.name ) {
					result = iotivity.OCSetDeviceInfo( { deviceName: settings.info.name } );
					if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
						reject( "OicDevice: OCSetDeviceInfo() failed" );
					}
				}

				// Device platformID and manufacturerName must be present if we're to call
				// OCSetPlatformInfo
				if ( settings.info.uuid && settings.info.manufacturerName ) {
					var index,
						platformInfo = {},

						// Mapping of fields from info to OCPlatformInfo
						fieldMap = {
							uuid: "platformID",
							manufacturerName: "manufacturerName",
							manufacturerUrl: "manufacturerUrl",
							platformVersion: "platformVersion",
							manufactureDate: "dateOfManufacture",
							osVersion: "operatingSystemVersion",
							model: "modelNumber",
							firmwareVersion: "firmwareVersion",
							supportUrl: "supportUrl"
						};

					// Populate platformInfo
					for ( index in fieldMap ) {
						if ( settings.info[ index ] ) {
							platformInfo[ fieldMap[ index ] ] = settings.info[ index ];
						}
					}

					result = iotivity.OCSetPlatformInfo( platformInfo );
					if ( result !== iotivity.OCStackResult.OC_STACK_OK ) {
						reject( "OicDevice: OCSetPlatformInfo() failed" );
					}
				}
			}

			fulfill();
		}, this ) );
	}
} );

module.exports = OicDevice;
