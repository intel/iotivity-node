#include <stdio.h>
#include <ocstack.h>
#include <glib.h>

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
defaultCb( void *context, OCDoHandle handle, OCClientResponse *response ) {
	int index, index1;
	g_message( "client: defaultCb() has received a message:" );
	g_message( "client: response" );
	g_message( "client: ->addr" );
	g_message( "client: ->addr->size: %d", response->addr->size );
	g_message( "client: ->connType: %d", response->connType );
	g_message( "client: ->result: %d", response->result );
	g_message( "client: ->sequenceNumber: %d", response->sequenceNumber );
	g_message( "client: ->resJSONPayload: %s", response->resJSONPayload );
	g_message( "client: ->numRcvdVendorSpecificHeaderOptions: %d",
		response->numRcvdVendorSpecificHeaderOptions );

	return OC_STACK_DELETE_TRANSACTION;
}

static void
addCallback() {
	OCDoHandle handle;
	OCCallbackData cbData = {
		NULL,
		defaultCb,
		NULL
	};

	if ( OC_STACK_OK != OCDoResource(
			&handle,
			OC_REST_GET,
			"/a/light",
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

		addCallback();
		g_timeout_add( 100, runOCProcess, mainLoop );
		g_main_loop_run( mainLoop );
		OCStop();
		return 0;
	}

	g_warning( "client: OCInit() failed" );
	return 1;
}
