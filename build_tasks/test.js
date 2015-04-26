function maybeFail( grunt, command, code, callback ) {
	if ( code === 0 ) {
		callback( null );
	} else {
		grunt.fail.warn( "Failed to execute command: " + command );
		callback( code );
	}
}

module.exports = function( grunt ) {
	grunt.registerTask( "test", function() {
		var done = this.async();
			shelljs = require( "shelljs" ),
			async = require( "async" );

		async.series( [
			function( callback ) {
				var command = "TESTING=true npm install";

				grunt.log.subhead( "Running command: " + command );

				shelljs.exec(
					command,
					{
						async: true,
						silent: true
					},
					function( code, output ) {
						maybeFail( grunt, command, code, callback );
					} );
			},
			function( callback ) {
				var command = "npm test";

				grunt.log.subhead( "Running command: " + command );

				shelljs.exec( command, { async: true }, function( code, output ) {
					maybeFail( grunt, command, code, callback );
				} );
			},
			function( callback ) {
				var command = "npm install";

				grunt.log.subhead( "Running command: " + command );

				shelljs.exec(
					command,
					{
						async: true,
						silent: true
					},
					function( code, output ) {
						maybeFail( grunt, command, code, callback );
					} );
			}
		], function() {
			done();
		} );
	} );
};
