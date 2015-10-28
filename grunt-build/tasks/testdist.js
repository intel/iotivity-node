module.exports = function( grunt ) {

var spawn = require( "child_process" ).spawn,
	packageName = grunt.file.readJSON( "package.json" ).name;

grunt.task.registerTask( "testdist", "Test the distributed version", function() {
	var done = this.async();

	spawn( ( process.env.SHELL || "sh" ),
		[
			"-c",
			"./dist.sh -i " + ( grunt.option( "ci" ) ? "-n" : "" ) +
			"&& cd dist/" + packageName +
			"&& echo '*** Loading high-level API ***'" +
			"&& node ./index.js" +
			"&& echo '*** Loading low-level API ***'" +
			"&& node ./lowlevel.js"
		],
		{ stdio: "inherit" } )
		.on( "exit", function( code ) {
			done.apply( this, ( code === 0 ? [] : [ false ] ) );
		} );
} );

};
