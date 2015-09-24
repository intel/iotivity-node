module.exports = function( grunt ) {

var spawn = require( "child_process" ).spawn;

grunt.task.registerTask( "clangformat", "Format the C++ files", function() {
	var done = this.async();

	spawn( ( process.env.SHELL || "sh" ),
		[
			"-c",
			"find src -type f | while read; do " +
			"clang-format -style=Google \"$REPLY\" > \"$REPLY\".new && " +
			"mv \"$REPLY\".new \"$REPLY\" && echo \"File $REPLY formatted.\";" +
			"done"
		],
		{ stdio: "inherit" } )
		.on( "exit", function( code ) {
			done.apply( this, ( code === 0 ? [] : [ false ] ) );
		} );
} );

};
