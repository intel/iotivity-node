var lightResource,
	onRequestCalls = 0,
	uuid = process.argv[ 2 ],
	utils = require( "../../assert-to-console" ),
	device = require( "../../../index" )(),
	settings = {
		role: "server"
	};

console.log( JSON.stringify( { assertionCount: 5 } ) );

function cleanup() {
	utils.assert( "ok", onRequestCalls > 0, "Server: device.server.onrequest has been called" );
	utils.assert( "strictEqual", lightResource.properties.increment, 11,
		"Exactly 11 notifications were delivered (the last one with noObservers)" );
	console.log( JSON.stringify( { killPeer: true } ) );
	process.exit( 0 );
}

function lightResourceOnRequest( request ) {
	if ( request.type === "retrieve" || request.type === "observe" ) {
		request.sendResponse( null );
	}
}

function testOnRequest() {
	onRequestCalls++;
}

// After we've sent out ten notifications, we wait for the client to remove itself from the list of
// observers, and then we send out a final notification which we expect to fail in a certain way.
function assertNotifyFailure() {
	var waitForObserverListToEmpty = setInterval( function() {
		if ( device.server._interestedObservers.length === 0 ) {
			clearInterval( waitForObserverListToEmpty );
			lightResource.properties.increment++;
			device.server.notify( lightResource ).catch(
				function( error ) {
					utils.assert( "strictEqual", error.noObservers, true,
						"Server: device.server.notify() fails with noObservers when there are " +
						"no observers" );
					cleanup();
				} );
		}
	}, 500 );
}

device.configure( settings ).then(
	function() {
		utils.assert( "ok", true, "device.configure() successful" );

		device.server.registerResource( {
			id: { path: "/a/" + uuid },
			deviceId: uuid,
			resourceTypes: [ "core.light" ],
			interfaces: [ "oic.if.baseline" ],
			discoverable: true,
			observable: true,
			properties: { increment: 0 }
		} ).then(
			function( resource ) {
				utils.assert( "ok", true, "registerResource() successful" );
				lightResource = resource;
				device.server.addEventListener( "request", lightResourceOnRequest );
				device.server.onrequest = testOnRequest;
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

// Notify ten times and then stop this interval
var notifyIntervalId = setInterval( function() {
	var haveObservers = false;

	if ( !lightResource ) {
		return;
	}

	device.server.notify( lightResource, "update", [ "increment" ] ).then(
		function() {

			haveObservers = true;
			lightResource.properties.increment++;
			if ( lightResource.properties.increment === 10 ) {

				// After ten notifications we wait for the list of observers to go empty so we can
				// assert that notify() fails with noObservers: true
				clearInterval( notifyIntervalId );
				assertNotifyFailure();
			}
		},
		function( error ) {
			if ( haveObservers ) {
				utils.die( "notify() failed with " + error + " and result " + error.result );
			}
		} );
}, 1000 );
