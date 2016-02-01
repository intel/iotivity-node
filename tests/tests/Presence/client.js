var result,
	uuid = process.argv[ 2 ],
	countdownToCleanup = 0,
	processCallCount = 0,
	processLoop = null,
	iotivity = require( "../../../lowlevel" ),
	testUtils = require( "../../utils" )( iotivity );

function cleanup() {
	var result;

	if ( processLoop ) {
		clearInterval( processLoop );
		processLoop = null;
	}

	testUtils.assert( "ok", true, "Client: OCProcess succeeded " + processCallCount + " times" );

	result = iotivity.OCStop();
	if ( testUtils.stackOKOrDie( "Client", "OCStop", result ) ) {
		console.log( JSON.stringify( { killPeer: true } ) );
		process.exit( 0 );
	}
}

function maybeCleanup() {
	countdownToCleanup++;

	if ( countdownToCleanup === 2 ) {
		cleanup();
	}
}

// Tell the server to turn presence on/off
function requestServerPresence( destination, turnOnPresence ) {
	var handleReceptacle = {},
		result = iotivity.OCDoResource(
			handleReceptacle,
			iotivity.OCMethod.OC_REST_PUT,
			"/a/" + uuid,
			destination,
			{
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: { presence: turnOnPresence }
			},
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				testUtils.assert( "deepEqual",
					response.payload,
					{
						type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
						values: { result: "ok" }
					},
					"Client: Received correct response to PUT request" );

				// We know that turning off the presence is the last thing this test does. Still,
				// we need to wait both for the OC_PRESENCE_TRIGGER_DELETE (captured in
				// trackPresence() below, and for this response. Only when they have both completed
				// can we clean up.
				if ( turnOnPresence === false ) {
					maybeCleanup();
				}

				return iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;
			},
			null );

	testUtils.stackOKOrDie( "Client", "OCDoResource(PUT request presence " +
		( turnOnPresence ? "on" : "off" ) + ")", result );
}

function trackPresence( destination ) {
	var presenceHandleReceptacle = {},

		// We perform a fixed sequence of steps, and the value of presenceSequence tells us which
		// step we're currently performing:
		// 0 - the result of turning on presence on the server
		// 1 - the result of turning off presence on the server
		presenceSequence = 0,
		presenceResult = iotivity.OCDoResource(
			presenceHandleReceptacle,
			iotivity.OCMethod.OC_REST_PRESENCE,
			iotivity.OC_RSRVD_PRESENCE_URI,
			destination,
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {
				var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

				if ( response && response.payload ) {
					testUtils.assert( "strictEqual",
						testUtils.lookupEnumValueName( "OCPayloadType",
							response.payload.type ),
						"PAYLOAD_TYPE_PRESENCE",
						"Client: Response to PRESENCE request has payload of type PRESENCE" );
					if ( presenceSequence === 0 ) {
						testUtils.assert( "strictEqual",
							testUtils.lookupEnumValueName( "OCPresenceTrigger",
								response.payload.trigger ),
							"OC_PRESENCE_TRIGGER_CREATE",
							"Client: Response to PRESENCE request has trigger of type CREATE" );
						requestServerPresence( destination, false );
						presenceSequence++;
					} else if ( presenceSequence === 1 ) {
						testUtils.assert( "strictEqual",
							testUtils.lookupEnumValueName( "OCPresenceTrigger",
								response.payload.trigger ),
							"OC_PRESENCE_TRIGGER_DELETE",
							"Client: Response to PRESENCE request has trigger of type DELETE" );

						// We have completed the presence sequence, but the response to the PUT
						// request asking for the presence to be stopped may still be outstanding.
						// See requestServerPresence() for the other shoe. Thus, we don't clean up
						// outright, but we mark off another step towards that end.
						maybeCleanup();
					}
				}

				return returnValue;
			},
			null );
	testUtils.stackOKOrDie( "Client", "OCDoResource(presence tracking)", presenceResult );
	requestServerPresence( destination, true );
}

console.log( JSON.stringify( { assertionCount: 14 } ) );

// Initialize
result = iotivity.OCInit( null, 0, iotivity.OCMode.OC_CLIENT );
testUtils.stackOKOrDie( "Client", "OCInit", result );

// Set up OCProcess loop
processLoop = setInterval( function() {
	var processResult = iotivity.OCProcess();

	if ( processResult === iotivity.OCStackResult.OC_STACK_OK ) {
		processCallCount++;
	} else {
		testUtils.stackOKOrDie(
			"Client",
			"OCProcess(after " + processCallCount + " successful calls)",
			processResult );
	}
}, 100 );

function doDiscovery() {
	var handleReceptacle = {},
		result = iotivity.OCDoResource(
			handleReceptacle,
			iotivity.OCMethod.OC_REST_DISCOVER,
			iotivity.OC_MULTICAST_DISCOVERY_URI,
			null,
			null,
			iotivity.OCConnectivityType.CT_DEFAULT,
			iotivity.OCQualityOfService.OC_HIGH_QOS,
			function( handle, response ) {

				// We retain the discovery callback until we've found the resource with the uuid.
				var returnValue = iotivity.OCStackApplicationResult.OC_STACK_KEEP_TRANSACTION;

				if ( testUtils.findResource( response, uuid ) ) {

					// We've successfully completed the test so let's move to the next step.
					testUtils.assert( "ok", true, "Client: Resource found" );
					returnValue = iotivity.OCStackApplicationResult.OC_STACK_DELETE_TRANSACTION;

					// We know where the resource is so let's do the presence dance.
					trackPresence( response.addr );
				}

				return returnValue;
			},
			null );
	testUtils.stackOKOrDie( "Client", "OCDoResource(discovery)", result );
}

doDiscovery();
