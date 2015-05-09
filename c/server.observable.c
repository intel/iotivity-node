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
		g_warning( "server: OCProcess() did not return OC_STACK_OK - quitting main loop" );
		g_main_loop_quit( mainLoop );
		return G_SOURCE_REMOVE;
	}
}

static OCEntityHandlerResult
handleLight1( OCEntityHandlerFlag flag, OCEntityHandlerRequest *request ) {
	g_message( "server: handleLight1: Entering" );
	return OC_EH_OK;
}

int
main( int argc, char **argv ) {
	GMainLoop *mainLoop = g_main_loop_new( NULL, FALSE );
	OCResourceHandle handle;

	if ( OC_STACK_OK == OCInit( NULL, 0, OC_SERVER ) ) {
		g_message( "server: OCInit() succeeded" );

		if ( OC_STACK_OK == OCCreateResource(
			&handle,
			"core.light",
			"oc.mi.def",
			"/a/light",
			handleLight1,
			OC_DISCOVERABLE | OC_OBSERVABLE ) ) {

			g_message( "server: OCCreateResource() succeeded" );

			g_timeout_add( 100, runOCProcess, mainLoop );
			OCStartPresence( 0 );
			g_main_loop_run( mainLoop );
			OCStop();
			return 0;
		}
	}

	g_warning( "server: Startup failed" );
	return 1;
}
