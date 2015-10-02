module.exports = function( grunt ) {

var spawn = require( "child_process" ).spawn;

grunt.task.registerTask( "testdist", "Test the distributed version", function() {
	var done = this.async();

	spawn( ( process.env.SHELL || "sh" ),
		[
			"-c",
			"./dist.sh -i " + ( grunt.option( "ci" ) ? "-n" : "" ) +
			"&& cd dist/iotivity && node ./index.js && node ./lowlevel.js"
		],
		{ stdio: "inherit" } )
		.on( "exit", function( code ) {
			done.apply( this, ( code === 0 ? [] : [ false ] ) );
		} );
} );

};
