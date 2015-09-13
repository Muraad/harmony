{ 'targets': [
    {
        'target_name': 'protobuf',
        'type': 'static_library',
        'sources': [
            "<!@(find protobuf-2.5.0/src/google/protobuf -maxdepth 1 '(' -name '*.h' -o -name '*.cc' ')')",
            "<!@(find protobuf-2.5.0/src/google/protobuf/stubs -name '*.h' -o -name '*.cc')",
            "<!@(find protobuf-2.5.0/src/google/protobuf/io -name '*.h' -o -name '*.cc')",
        ],

        'sources/': [
            # exclude the testing and benchmarking sources
            ['exclude', 'unittest'],
            ['exclude', 'test_util'],
        ],

        'defines': [
        ],
        'cflags': [
		#	'-Dgoogle=__g__',
		#	'-Dprotobuf=__t__',
			'-Wwrite-strings',
			'-Woverloaded-virtual',
			'-Wno-sign-compare',
			'-Wno-unused-local-typedefs',
        ],
        'conditions': [
        ],
        'all_dependent_settings': {
            'include_dirs': [
                'protobuf-2.5.0/src',
            ]
        },
        'include_dirs': [
			'protobuf-2.5.0',
            'protobuf-2.5.0/src',
        ],
    },
]}
