var intervalId,
	iotivity = require( "iotivity" ),
	handle = {};

iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 100 );

iotivity.OCCreateResource(
	handle,
	"light",
	"oc.mi.def",
	"/light/1",
	function( flag, request ) {
		console.log( "OCCreateResource() handler: Entering" );
		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE
);

process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );
	clearInterval( intervalId );
	iotivity.OCStop();
	process.exit( 0 );
} );
