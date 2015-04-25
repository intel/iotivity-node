module.exports = function( grunt ) {
	"use strict";

	grunt.config.init( {
		bowercopy: {
			options: {
				clean: true,
				destPrefix: "external"
			},
			iotivity: {
				files: {
					"iotivity/resource/csdk": "iotivity/resource/csdk",
					"iotivity/resource/oc_logger/c": "iotivity/resource/oc_logger/c",
					"iotivity/resource/oc_logger/include": "iotivity/resource/oc_logger/include",
					"iotivity/extlibs/cjson": "iotivity/extlibs/cjson"
				}
			}
		}
	} );

	// grunt plugins
	require( "load-grunt-tasks" )( grunt );
	grunt.loadTasks( "build_tasks" );
};
