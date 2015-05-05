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
		g_warning( "OCProcess() did not return OC_STACK_OK - quitting main loop\n" );
		g_main_loop_quit( mainLoop );
		return G_SOURCE_REMOVE;
	}
}

int
main( int argc, char **argv ) {
	GMainLoop *mainLoop = g_main_loop_new( NULL, FALSE );

	if ( OC_STACK_OK == OCInit( NULL, 0, OC_SERVER ) ) {
		g_timeout_add_seconds( 2, runOCProcess, mainLoop );

		OCStartPresence( 0 );

		g_main_loop_run( mainLoop );
		OCStop();
		return 0;
	}

	g_warning( "OCInit() failed\n" );
	return 1;
}
