{
	"variables": {
		"externalOCTBStack": "<!(node -p \"( process.env.OCTBSTACK_CFLAGS && process.env.OCTBSTACK_LIBS ) ? 'true' : 'false';\")",
		"internalOCTBStack_include_dirs": [
			"<(module_root_dir)/iotivity-installed/include"
		],
	},

	"target_defaults": {
		"include_dirs": [
			"<!(node -e \"require( 'nan' );\")",
			"<(module_root_dir)/src"
		],
		"conditions": [

			[ "'<(externalOCTBStack)'=='true'", {

				# When building externally we simply follow the CFLAGS/LIBS

				"libraries": [ "<!@(node -p \"process.env.OCTBSTACK_LIBS\")" ],
				"cflags": [ "<!@(node -p \"process.env.OCTBSTACK_CFLAGS\")" ],
				"xcode_settings": {
					"OTHER_CFLAGS": [ "<!@(node -p \"process.env.OCTBSTACK_CFLAGS\")" ]
				}
			}, {

				# When building internally, we use pre-defined CFLAGS/LIBS, trusting that the CSDK
				# will be built successfully

				"defines": [ "ROUTING_EP" ],
				"include_dirs+": [ '<@(internalOCTBStack_include_dirs)' ],
				"conditions": [

					# Windows-specific way of adding libraries

					[ "OS=='win'", {
						"libraries": [
							"<(module_root_dir)/iotivity-installed/lib/octbstack.lib"
						]
					}, {

					# Generic way that works for POSIX
						"libraries": [
							"-L<(module_root_dir)/iotivity-installed/lib",
							"-Wl,-rpath <(module_root_dir)/iotivity-installed/lib",
							"-loctbstack"
						]
					} ],

					[ "OS=='mac'", {

						# OSX needs some more libraries

						"libraries+": [
							"-lconnectivity_abstraction",
							"-lcoap",
							"-lc_common",
							"-lroutingmanager",
							"-llogger",
							"-locsrm"
						]
					} ]
				]
			} ],

			# OSX quirk

			[ "OS=='mac'", {
				"xcode_settings": { "OTHER_CFLAGS": [ '-std=c++11' ] }
			} ]
		],
		"cflags_cc": [ '-std=c++11' ],
	},

	"targets": [
		{
			"target_name": "csdk",
			"type": "none",
			"conditions": [
				[ "'<(externalOCTBStack)'=='false'", {
					"actions": [ {
						"action_name": "build",
						"inputs": [""],
						"outputs": ["iotivity-installed"],
						"action": [
							"node",
							"build-scripts/build-csdk.js",
							"<!@(node -p \"process.env.npm_config_debug === 'true' ? '--debug' : '';\")"
						],
						"message": "Building CSDK"
					} ],
					"conditions": [

						# On Windows we need to copy the dll next to the nodejs module
						[ "OS=='win'", {
							"copies": [ {
								"destination": "<(PRODUCT_DIR)",
								"files": [
									"<(module_root_dir)/iotivity-installed/lib/octbstack.dll"
								]
							} ]
						} ]
					]
				} ]
			]
		},
		{
			"target_name": "generateconstants",
			"type": "none",
			"actions": [ {
				"action_name": "generateconstants",
				"message": "Generating constants",
				"outputs": ["generated/constants.cc"],
				"conditions": [
					[ "'<(externalOCTBStack)'=='false'", {
						"inputs": ["iotivity-installed"],
						"action": [
							"node",
							"build-scripts/generate-constants.js",
							'<@(internalOCTBStack_include_dirs)'
						]
					}, {
						"inputs": [""],
						"action": [
							"node",
							"build-scripts/generate-constants.js",
							"-c",
							"<!@(node -p \"process.env.OCTBSTACK_CFLAGS\")"
						]
					} ]
				]
			} ],
			"dependencies": [ "csdk" ]
		},
		{
			"target_name": "generateenums",
			"type": "none",
			"actions": [ {
				"action_name": "generateenums",
				"message": "Generating enums",
				"outputs": ["generated/enums.cc"],
				"conditions": [
					[ "'<(externalOCTBStack)'=='false'", {
						"inputs": ["iotivity-installed"],
						"action": [
							"node",
							"build-scripts/generate-enums.js",
							'<@(internalOCTBStack_include_dirs)'
						]
					}, {
						"inputs": [""],
						"action": [
							"node",
							"build-scripts/generate-enums.js",
							"-c",
							"<!@(node -p process.env.OCTBSTACK_CFLAGS)"
						]
					} ]
				]
			} ],
			"dependencies": [ "csdk" ]
		},
		{
			"target_name": "generatefunctions",
			"type": "none",
			"actions": [ {
				"action_name": "generatefunctions",
				"message": "Generating functions",
				"outputs": ["generated/functions.cc"],
				"conditions": [
					[ "'<(externalOCTBStack)'=='false'", {
						"inputs": ["iotivity-installed"],
						"action": [
							"node",
							"build-scripts/generate-functions.js"
						]
					}, {
						"inputs": [""],
						"action": [
							"node",
							"build-scripts/generate-functions.js"
						]
					} ]
				]
			} ],
			"dependencies": [ "csdk" ]
		},
		{
			"target_name": "iotivity",
			"cflags+": [ "-pedantic" ],
			"sources": [
				"generated/constants.cc",
				"generated/enums.cc",
				"generated/functions.cc",
				"src/common.cc",
				"src/functions/entity-handler.cc",
				"src/functions/oc-create-delete-resource.cc",
				"src/functions/oc-do-resource.cc",
				"src/functions/oc-register-persistent-storage-handler.cc",
				"src/functions/oc-server-resource-utils.cc",
				"src/functions/oc-set-default-device-entity-handler.cc",
				"src/functions/simple.cc",
				"src/main.cc",
				"src/structures/handles.cc",
				"src/structures/oc-client-response.cc",
				"src/structures/oc-dev-addr.cc",
				"src/structures/oc-device-info.cc",
				"src/structures/oc-entity-handler-request.cc",
				"src/structures/oc-identity.cc",
				"src/structures/oc-payload.cc",
				"src/structures/oc-platform-info.cc",
				"src/structures/oc-rep-payload/to-c.cc",
				"src/structures/oc-rep-payload/to-js.cc"
			],
			"dependencies": [ "csdk", "generateconstants", "generateenums", "generatefunctions" ]
		}
	]
}
