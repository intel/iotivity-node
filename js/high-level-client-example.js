var device = require( "iotivity" )();

device.configure( {
	role: "client",
	connectionMode: "acked"
} );

device._client.addEventListener( "resourcefound", function( event ) {
	console.log( "resource found:\n" + JSON.stringify( event.resource, null, 4 ) );
} );

device._client.findResources();
