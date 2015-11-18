var observationCount = 0,
	device = require( "iotivity-node" )();

device.configure( {
	role: "client"
} ).then( function() {

	// Add a listener that will receive the results of the discovery
	device.addEventListener( "resourcefound", function( event ) {
		console.log( "Discovered resource(s) via the following event:\n" +
			JSON.stringify( event, null, 4 ) );

		// We've discovered the resource we were seeking.
		if ( event.resource.id.path === "/a/high-level-example" ) {
			console.log( "This is the resource we want to observe" );

			// Let's start observing the resource.
			device.on( "resourcechange", function( event ) {
				console.log( "Received resource change event:\n" +
					JSON.stringify( event, null, 4 ) );

				// Stop observing after having made 10 observations
				if ( ++observationCount >= 10 ) {
					device.cancelObserving( event.resource.id );
				}
			} );
			device.startObserving( event.resource.id );
		}
	} );
	device.findResources();
} );
