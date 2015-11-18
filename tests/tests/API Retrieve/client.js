var resourceFound = false,
	quitConditionCount = 0,
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	uuid = process.argv[ 2 ];

console.log( JSON.stringify( { assertionCount: 4 } ) );

function maybeQuit() {
	quitConditionCount++;

	if ( quitConditionCount >= 3 ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

device.configure( {
	role: "client"
} ).then(
	function() {
		utils.assert( "ok", true, "Client: device.configure() successful" );

		device.addEventListener( "resourcefound", function( event ) {

			if ( !resourceFound && event.resource.uri === "/a/" + uuid ) {
				resourceFound = true;
				utils.assert( "ok", true, "Client: Resource found" );

				device.retrieveResource( event.resource.id ).then(
					function( resource ) {
						utils.assert( "deepEqual", resource.properties, {
							"How many angels can dance on the head of a pin?": "As many as wanting."
						}, "Retrieved properties are as expected." );

						// We have retrieved the resource, so we have completed one step towards
						// concluding the test
						maybeQuit();
					},
					function( error ) {
						utils.die( "Failed to retrieve resource with " + error + " and result " +
							error.result );
					} );

				// We found the resource, so we have completed one step towards concluding the test
				maybeQuit();
			}
		} );

		device.findResources().then(
			function() {
				utils.assert( "ok", true, "Client: findResources() successful" );

				// We have completed the process of finding resources, so we have completed one
				// step towards concluding the test
				maybeQuit();
			},
			function( error ) {
				utils.die( "Client: findResources() failed with: " + error );
			} );
	},
	function( error ) {
		utils.die( "Client: device.configure() failed with: " + error );
	} );

