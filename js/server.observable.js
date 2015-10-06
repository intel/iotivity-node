var intervalId,
	handleReceptacle = {},
	observerIds = [],

	// This is the same value as server.get.js
	sampleUri = "/a/iotivity-node-observe-sample",
	iotivity = require( "iotivity-node/lowlevel" );

// Start iotivity and set up the processing loop
iotivity.OCInit( null, 0, iotivity.OCMode.OC_SERVER );

iotivity.OCSetDeviceInfo( { deviceName: "server.observable" } );
iotivity.OCSetPlatformInfo( {
	platformID: "server.observe.sample",
	manufacturerName: "iotivity-node"
} );

intervalId = setInterval( function() {
	iotivity.OCProcess();
}, 1000 );

require( "./mock-sensor" )().on( "change", function( data ) {
	console.log( "Sensor data has changed. " +
		( observerIds.length > 0 ?
			"Notifying " + observerIds.length + " observers." :
			"No observers in list." ) );
	if ( observerIds.length > 0 ) {
		iotivity.OCNotifyListOfObservers(
			handleReceptacle.handle,
			observerIds,
			{
				type: iotivity.OCPayloadType.PAYLOAD_TYPE_REPRESENTATION,
				values: data
			},
			iotivity.OCQualityOfService.OC_HIGH_QOS );
	}
} );

// Create a new resource
iotivity.OCCreateResource(

	// The bindings fill in this object
	handleReceptacle,

	"core.fan",
	iotivity.OC_RSRVD_INTERFACE_DEFAULT,
	sampleUri,
	function( flag, request ) {
		var observerIdIndex;

		console.log( "Entity handler called with flag = " + flag + " and the following request:" );
		console.log( JSON.stringify( request, null, 4 ) );

		if ( flag & iotivity.OCEntityHandlerFlag.OC_OBSERVE_FLAG ) {
			if ( request.obsInfo.obsId !== 0 ) {
				if ( request.obsInfo.action === iotivity.OCObserveAction.OC_OBSERVE_REGISTER ) {

					// Add new observer to list.
					observerIds.push( request.obsInfo.obsId );
				} else if ( request.obsInfo.action ===
						iotivity.OCObserveAction.OC_OBSERVE_DEREGISTER ) {

					// Remove requested observer from list.
					observerIdIndex = observerIds.indexOf( request.obsInfo.obsId );
					if ( observerIdIndex >= 0 ) {
						observerIds.splice( observerIdIndex, 1 );
					}
				}
			}

			iotivity.OCDoResponse( {
				requestHandle: request.requestHandle,
				resourceHandle: request.resource,
				ehResult: iotivity.OCEntityHandlerResult.OC_EH_OK,
				payload: null,
				resourceUri: sampleUri,
				sendVendorSpecificHeaderOptions: []
			} );
		}

		return iotivity.OCEntityHandlerResult.OC_EH_OK;
	},
	iotivity.OCResourceProperty.OC_DISCOVERABLE |
	iotivity.OCResourceProperty.OC_OBSERVABLE );

// Exit gracefully when interrupted
process.on( "SIGINT", function() {
	console.log( "SIGINT: Quitting..." );

	// Tear down the processing loop and stop iotivity
	clearInterval( intervalId );
	iotivity.OCDeleteResource( handleReceptacle.handle );
	iotivity.OCStop();

	// Exit
	process.exit( 0 );
} );
