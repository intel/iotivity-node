{
	"variables": {
		"externalOCTBStack": '<!(if test "x${OCTBSTACK_CFLAGS}x" != "xx" -a "x${OCTBSTACK_CFLAGS}x" != "xx"; then echo true; else echo false; fi)',

		# Used when externalOCTBStack is false
		"internal_octbstack_cflags": [
			'<!@(echo "-I$(pwd)/deps/iotivity/include/iotivity/resource/csdk/stack/include")',
			'<!@(echo "-I$(pwd)/deps/iotivity/include/iotivity/resource/csdk/ocrandom/include")',
			'<!@(echo "-I$(pwd)/deps/iotivity/include/iotivity/resource/c_common")',
			'-DROUTING_EP',
			'-DTCP_ADAPTER'
		]
	},

	"conditions": [

		# Build dlopennow when testing so we can make sure the library has all the symbols it needs
		[ "'<!(echo $TESTING)'=='true'", {
			"targets+": [
				{
					"target_name": "dlopennow",
					"sources": [ "tests/dlopennow.cc" ],
					"include_dirs": [
					"<!(node -e \"require('nan')\")"
					]
				}
			]
		} ]
	],

	"target_defaults": {
		"include_dirs": [
			"<!(node -e \"require('nan')\")",
			"<!@(echo \"$(pwd)/src\")"
		],
		"conditions": [

			# Platform-independent conditions

			[ "'<(externalOCTBStack)'=='true'", {
				"libraries": [ '<!@(echo "$OCTBSTACK_LIBS")' ],
				"cflags": [ '<!@(echo "$OCTBSTACK_CFLAGS")' ],
				"xcode_settings": {
					"OTHER_CFLAGS": [ '<!@(echo "$OCTBSTACK_CFLAGS")' ]
				}
			}, {
				"libraries": [
					'<!@(echo "-L$(pwd)/deps/iotivity/lib")',
					'-loctbstack',
					'<!@(echo "-Wl,-rpath $(pwd)/deps/iotivity/lib")'
				],
				"cflags": [ '<(internal_octbstack_cflags)' ],
				"xcode_settings": {
					"OTHER_CFLAGS": [ '<(internal_octbstack_cflags)' ]
				}
			} ],

			# OSX-specific conditions

			[ "OS=='mac' and '<(externalOCTBStack)'=='false'", {
				"libraries+": [
					"-lconnectivity_abstraction",
					"-lcoap",
					"-lc_common",
					"-lroutingmanager",
					"-locsrm"
				]
			} ],
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
						"outputs": ["deps/iotivity"],
						"action": [
							"sh",
							"./build-csdk.sh",
							'<!@(if test "x${npm_config_debug}x" != "xtruex"; then echo ""; else echo "--debug"; fi)'
						],
						"message": "Building CSDK"
					} ]
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
						"inputs": ["deps/iotivity"],
						"action": [
							"sh",
							"./generate-constants.sh",
							'<(internal_octbstack_cflags)'
						]
					}, {
						"inputs": [""],
						"action": [
							"sh",
							"./generate-constants.sh",
							'<!@(echo "$OCTBSTACK_CFLAGS")'
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
						"inputs": ["deps/iotivity"],
						"action": [
							"sh",
							"./generate-enums.sh",
							'<(internal_octbstack_cflags)'
						]
					}, {
						"inputs": [""],
						"action": [
							"sh",
							"./generate-enums.sh",
							'<!@(echo "$OCTBSTACK_CFLAGS")'
						]
					} ]
				]
			} ],
			"dependencies": [ "csdk" ]
		},
		{
			"target_name": "iotivity",
			"sources": [
				"generated/constants.cc",
				"generated/enums.cc",
				"src/functions.cc",
				"src/functions/oc-cancel.cc",
				"src/functions/oc-create-delete-resource.cc",
				"src/functions/oc-do-resource.cc",
				"src/functions/oc-do-response.cc",
				"src/functions/oc-notify.cc",
				"src/functions/oc-random.cc",
				"src/functions/oc-set-default-device-entity-handler.cc",
				"src/functions/simple.cc",
				"src/main.cc",
				"src/structures.cc",
				"src/structures/handles.cc",
				"src/structures/oc-client-response.cc",
				"src/structures/oc-dev-addr.cc",
				"src/structures/oc-entity-handler-response.cc",
				"src/structures/oc-header-option-array.cc",
				"src/structures/oc-payload.cc",
				"src/structures/oc-platform-info.cc",
				"src/structures/oc-sid.cc",
				"src/structures/string-primitive.cc"
			],
			"conditions": [
				[ "'<!(echo $TESTING)'=='true'", {
					"defines": [ "TESTING" ]
				} ]
			],
			"dependencies": [ "csdk", "generateconstants", "generateenums" ]
		}
	]
}
