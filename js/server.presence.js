var intervalId,
	handleReceptacle = {},

	// This is the same value as client.presence.js
	sampleUri = "/a/iotivity-node-presence-sample",
	iotivity = require( "iotivity-node/lowlevel" );

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
	sampleUri,
	function( flag, request ) {
		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE );

// Read keystrokes from stdin
var stdin = process.stdin;

stdin.setRawMode( true );
stdin.resume();
stdin.setEncoding( "utf8" );
stdin.on( "data", function( key ) {
	var result;

	switch ( key ) {

	case "p":
		result = iotivity.OCStartPresence( 0 );
		console.log( "OCStartPresence: " + result );
		break;

	case "s":
		result = iotivity.OCStopPresence();
		console.log( "OCStopPresence: " + result );
		break;

	// ^C
	case "\u0003":

		// Tear down the processing loop and stop iotivity
		clearInterval( intervalId );
		iotivity.OCDeleteResource( handleReceptacle.handle );
		iotivity.OCStopPresence();
		iotivity.OCStop();

		// Exit
		process.exit( 0 );
		break;

	default:
		return;
	}

	console.log( "Press 'p' to turn on presence\nPress 's' to turn off presence\n" +
		"Press Ctrl+C to exit" );
} );

console.log( "Press 'p' to turn on presence\nPress 's' to turn off presence\n" +
	"Press Ctrl+C to exit" );
