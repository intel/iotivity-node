module.exports = function( grunt ) {

grunt.registerTask( "default", [ "test" ] );

grunt.registerTask( "test", [ "jshint", "jscs", "testsuite", "testdist" ] );

grunt.registerTask( "format", [ "esformatter", "clangformat" ] );

};
