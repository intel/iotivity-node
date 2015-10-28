var lightResource,
	uuid = process.argv[ 2 ],
	utils = require( "../../assert-to-console" ),
	sensor = require( "../../../js/mock-sensor" )(),
	device = require( "../../../index" )(),
	settings = {
		role: "server",
		connectionMode: "acked"
	};

console.log( JSON.stringify( { assertionCount: 2 } ) );

function lightResourceOnRequest( request ) {
	if ( request.type === "retrieve" || request.type === "observe" ) {
		request.sendResponse( null );
	}
}

device.configure( settings ).then(
	function() {
		utils.assert( "ok", true, "device.configure() successful" );

		device.server.registerResource( {
			url: "/a/" + uuid,
			deviceId: uuid,
			connectionMode: device.settings.connectionMode,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			observable: true,
			properties: { someValue: 0, someOtherValue: "Helsinki" }
		} ).then(
			function( resource ) {
				utils.assert( "ok", true, "registerResource() successful" );
				lightResource = resource;
				device.server.addEventListener( "request", lightResourceOnRequest );
				console.log( JSON.stringify( { ready: true } ) );
			},
			function( error ) {
				utils.die( "registerResource() failed with " + error +
					" and result " + error.result );
			} );
	},
	function( error ) {
		utils.assert( "ok", false,
			"device.configure() failed with " + error + " and result " + error.result );
	} );

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
		device.server.notify( lightResource.id, "update", updates ).then(
			function() {},
			function( error ) {
				utils.die( "notify() failed with " + error + " and result " + error.result );
			} );
	}
} );
