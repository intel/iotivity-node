var resourceFound = false,
	uuid = process.argv[ 2 ],
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )();

console.log( JSON.stringify( { assertionCount: 6 } ) );

function discoverResources() {
	device.addEventListener( "resourcefound", function( event ) {
		var observationHandle,
			dummyHandler = function() {},
			resourceUpdate = function( event ) {
				if ( event.resource.properties.increment >= 9 ) {
					event.resource.removeEventListener( "update", resourceUpdate );
					utils.assert( "deepEqual", event.resource._observationHandle,
						observationHandle,
						"Client: Private resource observation handle is unchanged after " +
						"removing a handler" );
					event.resource.removeEventListener( "update", dummyHandler );
					utils.assert( "ok", !event.resource._observationHandle,
						"Client: Private resource observation handle is absent after " +
						"removing the last handler" );
				}
			};

		if ( !resourceFound && event.resource.id.path === "/a/" + uuid ) {
			resourceFound = true;
			utils.assert( "ok", !event.resource._observationHandle,
				"Client: Private resource observation handle is initially absent" );
			event.resource.addEventListener( "update", resourceUpdate );
			observationHandle = event.resource._observationHandle;
			event.resource.addEventListener( "update", dummyHandler );
			utils.assert( "deepEqual", event.resource._observationHandle, observationHandle,
				"Client: Private resource observation handle is unchanged after adding a second " +
				"handler" );
		}
	} );
	device.findResources().then(
		function() {
			utils.assert( "ok", true, "Client: findResources() successful" );
		},
		function( error ) {
			utils.die( "Client: findResources() failed with " + error +
				" and result " + error.result );
		} );
}

device.configure( {
	role: "client"
} ).then(
	function() {
		utils.assert( "ok", true, "Client: device.configure() successful" );

		discoverResources();
	},
	function( error ) {
		utils.assert( "ok", false, "Client: device.configure() failed with " + error );
	} );

