var observationCount = 0,
	device = require( "iotivity" )();

device.configure( {
	role: "client",
	connectionMode: "acked"
} );

// Add a listener that will receive the results of the discovery
device._client.addEventListener( "resourcefound", function( event ) {
	console.log( "Discovered resource(s) via the following event:\n" +
		JSON.stringify( event, null, 4 ) );

	// We've discovered the resource we were seeking.
	if ( event.resource.uri === "/a/high-level-example" ) {
		console.log( "This is the resource we want to observe" );

		// Let's start observing the resource.
		device._client.on( "resourcechange", function( event ) {
			console.log( "Received resource change event:\n" + JSON.stringify( event, null, 4 ) );

			// Stop observing after having made 10 observations
			if ( ++observationCount >= 10 ) {
				device._client.cancelObserving( event.resource.id );
			}
		} );
		device._client.startObserving( event.resource.id );
	}
} );
device._client.findResources();
