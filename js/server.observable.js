var intervalId,
	handle = {},

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-observe-sample",
	iotivity = require( "iotivity" ),
	currentData = {};

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetDeviceInfo( { deviceName: "server.observable" } );
iotivity.OCSetPlatformInfo( {
	platformID: "server.observe.sample",
	manufacturerName: "iotivity-node"
} );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

require( "./mock-sensor" )().on( "change", function( data ) {
	console.log( "Sensor data has changed. Notifying all observers." );
	currentData = data;
	iotivity.OCNotifyAllObservers( handle.handle, iotivity.OCQualityOfService.OC_HIGH_QOS );
} );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );

		iotivity.OCDoResponse( {
			requestHandle: request.requestHandle,
			resourceHandle: request.resource,
			ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
			payload: {
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: currentData
			},
			resourceUri: sampleUri,
			sendVendorSpecificHeaderOptions: []
		} );

		// By default we error out
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE );


// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );
