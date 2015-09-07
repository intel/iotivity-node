module.exports = function( grunt ) {
"use strict";

var path = require( "path" );

require( "load-grunt-config" )( grunt, {
	configPath: [
		path.join( __dirname, "grunt-build", "tasks", "options" ),
		path.join( __dirname, "grunt-build", "tasks" )
	],
	init: true
} );

require( "load-grunt-tasks" )( grunt );

};
