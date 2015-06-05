// Server that illustrates the action of the entity handler. client.observable.js contains a client
// that issues a request that causes the entity handler to fire.

var intervalId,
	counter = 0,
	iotivity = require( "iotivity" ),
	handle = {};

iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

intervalId = setInterval( function() {
	iotivity.OCProcess();
	counter = ( counter + 1 ) % 10;
	if ( !counter ) {
		iotivity.OCNotifyAllObservers( handle, iotivity.OCQualityOfService.OC_HIGH_QOS );
	}
}, 100 );

iotivity.OCCreateResource(
	handle,
	"core.light",
	"oc.mi.def",
	"/a/light",
	function( flag, request ) {
		var payload = JSON.stringify( {
				"href": {
					"rep": {
						"0": "random",
						"1": "int",
						"2": Math.round( Math.random() * 100 )
					}
				}
			} );

		console.log( "OCCreateResource() handler: Responding with " + payload );
		iotivity.OCDoResponse( {
			requestHandle: request.requestHandle.handle,
			resourceHandle: request.resource.handle,
			ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
			payload: payload,
			payloadSize: payload.length,
			numSendVendorSpecificHeaderOptions: 0,
			sendVendorSpecificHeaderOptions: [],
			resourceUri: 0,
			persistentBufferFlag: 0
		} );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE
);

process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );
	clearInterval( intervalId );
	iotivity.OCStop();
	process.exit( 0 );
} );
