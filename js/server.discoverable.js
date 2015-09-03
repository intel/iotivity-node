var intervalId,
	handleReceptacle = {},
	iotivity = require( "iotivity" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

iotivity.OCSetDeviceInfo( { deviceName: "server.discoverable" } );
iotivity.OCSetPlatformInfo( {
	platformID: "server.discoverable.sample",
	manufacturerName: "iotivity-node"
} );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	"/a/fan",
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );


// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handleReceptacle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );
