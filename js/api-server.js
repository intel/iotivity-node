var lightResource,
	sensor = require( "./mock-sensor" )(),
	iotivity = require( "iotivity" ),
	device = iotivity.OicDevice(),
	settings = {
		role: "server",
		connectionMode: "acked",
		info: {
			uuid: "INTEL"
		}
	};

device.configure( settings );

sensor.on( "change", function( newData ) {
	var index,
		updates = [];

	if ( !lightResource ) {
		return;
	}

	for ( index in newData ) {
		if ( newData[ index ] !== lightResource.properties[ index ] ) {
			lightResource.properties[ index ] = newData[ index ];
			updates.push( index );
		}
	}
	if ( updates.length > 0 ) {
		device._server.notify( lightResource.id, "update", updates );
	}
} );

function lightResourceOnRequest( request ) {
	console.log( "lightResourceOnRequest: request:\n" +
		JSON.stringify( request, null, 4 ) );
	if ( request.type === "retrieve" ) {
		request.sendResponse( null );
	}
}

if ( device._settings.info.uuid ) {
	device._server.registerResource( {
		url: "/light/ambience/blue",
		deviceId: device._settings.info.uuid,
		connectionMode: device._settings.connectionMode,
		resourceTypes: [ "Light" ],
		interfaces: [ "/oic/if/rw" ],
		discoverable: true,
		observable: true,
		properties: { someValue: 0, someOtherValue: "Helsinki" }
	} ).then(
		function( resource ) {
			lightResource = resource;
			resource._server.addEventListener( "request", lightResourceOnRequest );
		},
		function( error ) {
			throw error;
		} );
}
