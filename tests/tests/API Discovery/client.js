var quitConditionCount = 0,
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 3 } ) );

function maybeQuit() {
	quitConditionCount++;

	if ( quitConditionCount >= 2 ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

device.configure( {
	role: "client",
	connectionMode: "acked"
} ).then(
	function() {
		utils.assert( "ok", true, "Client: device.configure() successful" );

		device._client.addEventListener( "resourcefound", function( event ) {

			if ( event.resource.uri === "/a/" + uuid ) {
				utils.assert( "ok", true, "Client: Resource found" );

				// We found the resource, so we have completed one step towards concluding the test
				maybeQuit();
			}
		} );

		device._client.findResources().then(
			function() {
				utils.assert( "ok", true, "Client: findResources() successful" );
				maybeQuit();
			},
			function( error ) {
				utils.die( "Client: findResources() failed with: " + error );
			} );
	},
	function( error ) {
		utils.die( "Client: device.configure() failed with: " + error );
	} );

