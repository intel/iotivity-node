var intervalId,
	handle = {},

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-get-sample",
	iotivity = require( "iotivity" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetDeviceInfo( { deviceName: "server.get" } );
iotivity.OCSetPlatformInfo( {
	platformID: "server.get.sample",
	manufacturerName: "iotivity-node"
} );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

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

		// If we find the magic question in the request, we return the magic answer
		if ( request && request.payload && request.payload.values &&
				request.payload.values.question ===
					"How many angels can dance on the head of a pin?" ) {

			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: {
					type: 4,
					values: {
						"answer": "As many as wanting."
					}
				},
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );

			return iotivity.OCEntityHandlerResult.OC_EH_OK;
		}

		// By default we error out
		return iotivity.OCEntityHandlerResult.OC_EH_ERROR;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );


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
