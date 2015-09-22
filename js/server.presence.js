var intervalId,
	handleReceptacle = {},

	// This is the same value as client.presence.js
	sampleUri = "/a/iotivity-node-presence-sample",
	interruptSequence = 0,
	iotivity = require( "iotivity/lowlevel" );

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

console.log( "The first SIGINT will start presence.\nThe second SIGINT will stop presence.\n" +
	"The third SIGINT will quit the server." );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	if ( interruptSequence === 0 ) {
		console.log( "\nSIGINT: Starting presence..." );

		iotivity.OCStartPresence( 0 );
	} else if ( interruptSequence === 1 ) {
		console.log( "\nSIGINT: Stopping presence..." );

		iotivity.OCStopPresence();
	} else {
		console.log( "\nSIGINT: Quitting..." );

		// Tear down the processing loop and stop iotivity
		clearInterval( intervalId );
		iotivity.OCDeleteResource( handleReceptacle.handle );
		iotivity.OCStopPresence();
		iotivity.OCStop();

		// Exit
		process.exit( 0 );
	}
	interruptSequence++;
} );
