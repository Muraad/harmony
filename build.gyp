{
	#'conditions': [
	#	['OS=="android"', {
	#		'targets': [
	#			{
	#				'target_name': 'imcore_jni',
	#				'type': 'shared_library',
	#				'conditions': [],
	#				'dependencies': [
	#					'libimcore',
	#				],
	#				'sources': [
	#					'<!@(find java -name "*.cc" -o -name "*.cxx" -o -name "*.h")',
	#				],
	#				'include_dirs': [
	#					'include',
	#					'src',
	#				],
	#			},
	#		],
	#	}],
	#],
    'targets': [
        #{
        #    'target_name': 'libtim',
        #    'type': 'static_library',
        #    'conditions': [],
        #    'dependencies': [
        #        'third_party/protobuf.gyp:protobuf',
        #    ],
        #    'sources': [
        #        '<!@(find include src net -name "*.cc" -o -name "*.h" -o -name "*.cpp" -o -name "*.c")',
        #    ],
        #    #'sources!': [
		#	#	'net/main.cc',
        #    #    '<!@(find net -name "*_c.cpp")',
		#	#	'<!@(find net/demo -name "*")',
        #    #],

        #    'include_dirs': [
        #        'include',
		#		'src',
		#		'src/pbgen',
		#		'third_party/rapidjson-v0.11-include',
        #    ],
        #    'all_dependent_settings': {
        #        'include_dirs': [
        #            'include',
        #        ],
        #    },
        #},
        {
            'target_name': 'test_main',
            'type': 'executable',
            'dependencies': [
                #'libtim',
                'third_party/http_parser.gyp:http_parser',
				'third_party/uv.gyp:libuv',
            ],
            'include_dirs': [
				'third_party/pt-1.4/',
				#'include',
				#'src',
                #'net',
            ],
			#'libraries': [
			#	'-luuid',
			#	'-lpthread',
			#	'-lrt',
			#	'-ldl',
			#	'/usr/local/lib64/libstdc++.a',
			#],
            'sources': [
				'<!@(find . -maxdepth 1 -name "*.cc" -o -name "*.h")',
            ]
        },
    ],
}
