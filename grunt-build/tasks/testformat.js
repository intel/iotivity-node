module.exports = function( grunt ) {

var spawn = require( "child_process" ).spawn,
	path = require( "path" ),
	shelljs = require( "shelljs" ),
	_ = require( "lodash" ),
	async = require( "async" ),
	jsFiles = grunt.file.expand.apply( grunt.file, require( "../js-files" ) );

function createFormatTarget( baseDirectory ) {
	shelljs.mkdir( baseDirectory );

	_.forEach( jsFiles, function( file ) {
		var destination = path.join( baseDirectory, path.dirname( file ) );
		shelljs.mkdir( "-p", destination );
		shelljs.cp( file, destination );
	} );

	shelljs.cp( "-R", "src", baseDirectory );
}

grunt.task.registerTask( "testformat", "Test formatting of source code", function() {
	var error = null,
		done = this.async();

	createFormatTarget( "baseline" );

	async.series( [
		function( callback ) {
			spawn( "grunt", [ "format" ], { stdio: "inherit" } )
				.on( "exit", function( code ) {
					if ( code ) {
						error = "grunt format failed";
					} else {
						createFormatTarget( "formatted" );
					}
					grunt.log.writeln( "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" );
					grunt.log.writeln( "grunt format subprocess complete" );
					callback( null );
				} );
		},
		function( callback ) {
			if ( error ) {
				async.setImmediate( function() {
					callback( null );
				} );
			} else {
				spawn( "diff",
					[
						"-Nru",
						"baseline",
						"formatted"
					],
					{ stdio: "inherit" } )
					.on( "exit", function( code ) {
						if ( code ) {
							error = "diff failed";
						}
						callback( null );
					} );
			}
		},
		function( callback ) {
			shelljs.rm( "-rf", "baseline", "formatted" );
			callback( error );
		}
	], function( error ) {
		done.apply( this, ( error ? [ false ] : [] ) );
	} );
} );

};
