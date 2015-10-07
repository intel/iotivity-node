var lightResource,
	sensor = require( "./mock-sensor" )(),
	device = require( "iotivity-node" )(),
	settings = {
		role: "server",
		connectionMode: "acked",
		info: {
			uuid: "INTEL-server",
			name: "api-server-example",
			manufacturerName: "Intel",
			manufactureDate: "Wed Sep 23 10:04:17 EEST 2015",
			platformVersion: "0.9.2",
			firmwareVersion: "0.0.1",
			supportUrl: "http://example.com/"
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
	if ( request.type === "retrieve" || request.type === "observe" ) {
		request.sendResponse( null );
	}
}

if ( device._settings.info.uuid ) {
	device._server.registerResource( {
		url: "/a/high-level-example",
		deviceId: device._settings.info.uuid,
		connectionMode: device._settings.connectionMode,
		resourceTypes: [ "core.light" ],
		interfaces: [ "oic.if.baseline" ],
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
