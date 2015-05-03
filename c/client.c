#include <stdio.h>
#include <glib.h>
#include <ocstack.h>

static gboolean
runOCProcess( gpointer user_data ) {
	OCStackResult result = OCProcess();
	GMainLoop *mainLoop = ( GMainLoop * )user_data;

	if ( result == OC_STACK_OK ) {
		return G_SOURCE_CONTINUE;
	} else {
		g_warning( "OCProcess() did not return OC_STACK_OK - quitting main loop\n" );
		g_main_loop_quit( mainLoop );
		return G_SOURCE_REMOVE;
	}
}

static OCStackApplicationResult
defaultCb( void *context, OCDoHandle handle, OCClientResponse *response ) {
	g_message( "defaultCb() has received a message\n" );

	return OC_STACK_DELETE_TRANSACTION;
}

static void
addPresenceCallback() {
	OCDoHandle handle;
	OCCallbackData cbData = {
		NULL,
		defaultCb,
		NULL
	};

	if ( OC_STACK_OK != OCDoResource(
			&handle,
			OC_REST_PRESENCE,
			"coap://255.255.255.255:5683/a/led",
			NULL,
			NULL,
			OC_ALL,
			OC_NA_QOS,
			&cbData,
			NULL,
			0 ) ) {

		g_warning( "OCDoResource() failed\n" );
	}
}

int
main( int argc, char **argv ) {
	GMainLoop *mainLoop = g_main_loop_new( NULL, FALSE );

	if ( OC_STACK_OK == OCInit( NULL, 0, OC_CLIENT ) ) {
		addPresenceCallback();
		g_timeout_add_seconds( 2, runOCProcess, mainLoop );
		g_main_loop_run( mainLoop );
		OCStop();
		return 0;
	}

	g_warning( "OCInit() failed\n" );
	return 1;
}
