#include <stdio.h>
#include <string.h>
#include <ocstack.h>
#include <glib.h>
#include <sys/types.h>
#include <regex.h>

/*
 * To get notifications you need to call OCDoResource() with /oc/core to first discover the
 * resource for which you want notifications, and then you need to issue a second OCDoResource()
 * with the method set to OC_REST_OBSERVE, the absolute URL computed from the response to the first
 * OCDoResource() and the connectivity type returned in the connType field of the response to the
 * first OCDoResponse() call.
 *
 * Presumably, if you know the parameters for the second OCDoResource() call a priori, you can skip
 * the first OCDoResource() call and go straight for the notifications.
 */

static void dumpResponse( OCClientResponse *response ) {
	int index, index1;

	g_message( "client: response" );
	g_message( "client: ->addr" );
	g_message( "client: ->addr->size: %d", response->addr->size );
	g_message( "client: ->connType: %d", response->connType );
	g_message( "client: ->result: %d", response->result );
	g_message( "client: ->sequenceNumber: %d", response->sequenceNumber );
	g_message( "client: ->resJSONPayload: %s", response->resJSONPayload );
	g_message( "client: ->numRcvdVendorSpecificHeaderOptions: %d",
		response->numRcvdVendorSpecificHeaderOptions );
	for ( index = 0 ; index < response->numRcvdVendorSpecificHeaderOptions ; index++ ) {
		g_message( "client: ->rcvdVendorSpecificHeaderOptions[ %d ]", index );
		g_message( "client:   ->rcvdVendorSpecificHeaderOptions[ %d ].protocolID: %d\n",
			index, ( int )( response->rcvdVendorSpecificHeaderOptions[ index ].protocolID ) );
		g_message( "client:   ->rcvdVendorSpecificHeaderOptions[ %d ].optionID: %d\n",
		index, ( int )response->rcvdVendorSpecificHeaderOptions[ index ].optionID );
		g_message( "client:   ->rcvdVendorSpecificHeaderOptions[ %d ].optionLength: %d\n",
			index, response->rcvdVendorSpecificHeaderOptions[ index ].optionLength );
		for ( index1 = 0 ;
				index1 < response->rcvdVendorSpecificHeaderOptions[ index ].optionLength;
				index1++ ) {
			g_message( "response->rcvdVendorSpecificHeaderOptions[ %d ].optionData[ %d ]: %d\n",
				index, index1,
				response->rcvdVendorSpecificHeaderOptions[ index ].optionData[ index1 ] );
		}
	}
}

static gboolean
runOCProcess( gpointer user_data ) {
	OCStackResult result = OCProcess();
	GMainLoop *mainLoop = ( GMainLoop * )user_data;

	if ( result == OC_STACK_OK ) {
		return G_SOURCE_CONTINUE;
	} else {
		g_warning( "client: OCProcess() did not return OC_STACK_OK - quitting main loop" );
		g_main_loop_quit( mainLoop );
		return G_SOURCE_REMOVE;
	}
}

static OCStackApplicationResult
observeCb( void *context, OCDoHandle handle, OCClientResponse *response ) {
	g_message( "client: observeCb() has received a message:" );

	dumpResponse( response );

	return OC_STACK_KEEP_TRANSACTION;
}

static void
addObserveCallback( char *url, OCConnectivityType connType ) {
	OCDoHandle handle;
	OCCallbackData cbData = {
		NULL,
		observeCb,
		NULL
	};

	if ( OC_STACK_OK != OCDoResource(
			&handle,
			OC_REST_OBSERVE,
			url,
			NULL,
			NULL,
			connType,
			OC_HIGH_QOS,
			&cbData,
			NULL,
			0 ) ) {

		g_warning( "client: OCDoResource() failed" );
	} else {
		g_message( "client: OCDoResource() succeeded" );
	}
}

static OCStackApplicationResult
discoverCb( void *context, OCDoHandle handle, OCClientResponse *response ) {
	unsigned char a, b, c, d;
	uint16_t port;
	regex_t hrefRe;
	regmatch_t matches[ 2 ];
	char path[ MAX_URI_LENGTH ] = { 0 };
	char url[ MAX_URI_LENGTH ] = { 0 };

	dumpResponse( response );

	if ( OC_STACK_OK == OCDevAddrToIPv4Addr( response->addr, &a, &b, &c, &d ) ) {
		g_message( "client: OCDevAddrToIPv4Addr() succeeded: %d.%d.%d.%d\n", a, b, c, d );
	} else {
		g_warning( "client: OCDevAddrToIPv4Addr() failed\n" );
	}

	if ( OC_STACK_OK == OCDevAddrToPort( response->addr, &port ) ) {
		g_message( "client: OCDevAddrToPort() succeeded: %d\n", port );
	} else {
		g_warning( "client: OCDevAddrToPort() failed\n" );
	}

	if ( !regcomp( &hrefRe, "\"href\"[[:space:]]*:[[:space:]]*\"([^\"]*)[\"]", REG_EXTENDED ) ) {
		g_message( "href regex compilation succeeded" );
		if ( !regexec( &hrefRe, response->resJSONPayload, 2, matches, 0 ) ) {
			strncpy(
				path,
				&response->resJSONPayload[ matches[ 1 ].rm_so ],
				matches[ 1 ].rm_eo - matches[ 1 ].rm_so );
			g_snprintf( url, MAX_URI_LENGTH, "coap://%d.%d.%d.%d:%d%s", a, b, c, d, port, path );
			g_message( "href regex matching succeeded: url: %s", url );

			addObserveCallback( url, response->connType );

		} else {
			g_warning( "href regex matching failed" );
		}
	} else {
		g_warning( "href regex compilation failed" );
	}

	return OC_STACK_DELETE_TRANSACTION;
}

void
addDiscoverCallback() {
	OCDoHandle handle;
	OCCallbackData cbData = {
		NULL,
		discoverCb,
		NULL
	};

	if ( OC_STACK_OK != OCDoResource(
			&handle,
			OC_REST_GET,
			OC_MULTICAST_DISCOVERY_URI,
			NULL,
			NULL,
			OC_ALL,
			OC_LOW_QOS,
			&cbData,
			NULL,
			0 ) ) {

		g_warning( "client: OCDoResource() failed" );
	} else {
		g_message( "client: OCDoResource() succeeded" );
	}
}

int
main( int argc, char **argv ) {
	GMainLoop *mainLoop = g_main_loop_new( NULL, FALSE );

	if ( OC_STACK_OK == OCInit( NULL, 0, OC_CLIENT ) ) {
		g_message( "client: OCInit() succeeded" );

		addDiscoverCallback();
		g_timeout_add( 100, runOCProcess, mainLoop );
		g_main_loop_run( mainLoop );
		OCStop();
		return 0;
	}

	g_warning( "client: OCInit() failed" );
	return 1;
}
