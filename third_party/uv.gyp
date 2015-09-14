{
  'target_defaults': {

    'default_configuration': 'Release',
    #'configurations': {
    #  'Debug': {
    #    'defines': [ 'DEBUG', '_DEBUG' ],
    #    'cflags': [ '-g', '-O0', '-fwrapv' ],
    #    'xcode_settings': {
    #      'GCC_OPTIMIZATION_LEVEL': '0',
    #      'OTHER_CFLAGS': [ '-Wno-strict-aliasing' ],
    #    },
    #    'conditions': [
    #      ['OS != "win"', {
    #        'defines': [ 'EV_VERIFY=2' ],
    #      }],
    #    ]
    #  },
    #  'Release': {
    #    'defines': [ 'NDEBUG' ],
    #    'cflags': [
    #      '-O3',
    #      '-fstrict-aliasing',
    #      '-fomit-frame-pointer',
    #      '-fdata-sections',
    #      '-ffunction-sections',
    #    ],
    #  }
    #},
    'conditions': [
      ['OS in "freebsd linux openbsd solaris android"', {
        #'cflags': [ '-Wall', '-fvisibility=hidden'  ],
        #'cflags_cc': [ '-fno-rtti', '-fno-exceptions' ],
        'target_conditions': [
          ['_type=="static_library"', {
            'standalone_static_library': 1, # disable thin archive which needs binutils >= 2.19
          }],
        ],
        'conditions': [
          #[ 'OS=="linux"', {
          #  'cflags': [ '-ansi' ],
          #}],
          [ 'OS not in "solaris android"', {
            'cflags': [ '-pthread' ],
            'ldflags': [ '-pthread' ],
          }],
        ],
      }],
      #['OS=="mac"', {
      #  'xcode_settings': {
      #    'ALWAYS_SEARCH_USER_PATHS': 'NO',
      #    'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
      #    'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
      #                                              # (Equivalent to -fPIC)
      #    'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',        # -fno-exceptions
      #    'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
      #    'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
      #    # GCC_INLINES_ARE_PRIVATE_EXTERN maps to -fvisibility-inlines-hidden
      #    'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
      #    'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',      # -fvisibility=hidden
      #    'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
      #    'PREBINDING': 'NO',                       # No -Wl,-prebind
      #    'USE_HEADERMAP': 'NO',
      #    'OTHER_CFLAGS': [
      #      '-fstrict-aliasing',
      #    ],
      #    'WARNING_CFLAGS': [
      #      '-Wall',
      #      '-Wendif-labels',
      #      '-W',
      #      '-Wno-unused-parameter',
      #    ],
      #  },
      #  #'conditions': [
      #  #  ['target_arch=="x64"', {
      #  #    'xcode_settings': {'ARCHS': ['x86_64']},
      #  #  }],
      #  #],
      #  'target_conditions': [
      #    ['_type!="static_library"', {
      #      'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-search_paths_first']},
      #    }],
      #  ],
      #}],
    ],


    'conditions': [
      ['OS != "win"', {
        'defines': [
          '_LARGEFILE_SOURCE',
          '_FILE_OFFSET_BITS=64',
        ],
        'conditions': [
          ['OS not in "solaris android"', {
            'cflags': [ '-pthread' ],
          }],
        ],
      }],
    ],
    'xcode_settings': {
      'WARNING_CFLAGS': [ '-Wall', '-Wextra', '-Wno-unused-parameter' ],
      'OTHER_CFLAGS': [ '-g', '--std=gnu89', '-pedantic' ],
    }
  },

  'targets': [
    {
      'target_name': 'libuv',
      'type': 'static_library',
      'include_dirs': [
        'libuv/include',
        'libuv/src/',
      ],
      'direct_dependent_settings': {
        'include_dirs': [ 'libuv/include' ],
        'conditions': [
          ['OS != "win"', {
            'defines': [
              '_LARGEFILE_SOURCE',
              '_FILE_OFFSET_BITS=64',
            ],
          }],
          ['OS in "mac ios"', {
            'defines': [ '_DARWIN_USE_64_BIT_INODE=1' ],
          }],
          ['OS == "linux"', {
            'defines': [ '_POSIX_C_SOURCE=200112' ],
          }],
        ],
      },
      'sources': [
        'libuv/include/uv.h',
        'libuv/include/tree.h',
        'libuv/include/uv-errno.h',
        'libuv/include/uv-threadpool.h',
        'libuv/include/uv-version.h',
        'libuv/src/fs-poll.c',
        'libuv/src/heap-inl.h',
        'libuv/src/inet.c',
        'libuv/src/queue.h',
        'libuv/src/threadpool.c',
        'libuv/src/uv-common.c',
        'libuv/src/uv-common.h',
        'libuv/src/version.c'
      ],
      'conditions': [
        [ 'OS!="win"', { # Not Windows i.e. POSIX
          'cflags': [
          #  '-g',
            #'--std=gnu89',
            '--std=c99',
          #  '-pedantic',
          #  '-Wall',
          #  '-Wextra',
            '-Wno-unused-parameter',
          ],
          'sources': [
            'libuv/include/uv-unix.h',
            'libuv/include/uv-linux.h',
            'libuv/include/uv-sunos.h',
            'libuv/include/uv-darwin.h',
            'libuv/include/uv-bsd.h',
            'libuv/include/uv-aix.h',
            'libuv/src/unix/async.c',
            'libuv/src/unix/atomic-ops.h',
            'libuv/src/unix/core.c',
            'libuv/src/unix/dl.c',
            'libuv/src/unix/fs.c',
            'libuv/src/unix/getaddrinfo.c',
            'libuv/src/unix/getnameinfo.c',
            'libuv/src/unix/internal.h',
            'libuv/src/unix/loop.c',
            'libuv/src/unix/loop-watcher.c',
            'libuv/src/unix/pipe.c',
            'libuv/src/unix/poll.c',
            'libuv/src/unix/process.c',
            'libuv/src/unix/signal.c',
            'libuv/src/unix/spinlock.h',
            'libuv/src/unix/stream.c',
            'libuv/src/unix/tcp.c',
            'libuv/src/unix/thread.c',
            'libuv/src/unix/timer.c',
            'libuv/src/unix/tty.c',
            'libuv/src/unix/udp.c',
          ],
          'link_settings': {
            'libraries': [ '-lm' ],
            'conditions': [
              ['OS != "solaris" and OS != "android"', {
                'ldflags': [ '-pthread' ],
              }],
            ],
          },
        }],
        [ 'OS in "linux mac ios android"', {
          'sources': [ 'libuv/src/unix/proctitle.c' ],
        }],
        [ 'OS in "mac ios"', {
          'sources': [
            'libuv/src/unix/darwin.c',
            'libuv/src/unix/fsevents.c',
            'libuv/src/unix/darwin-proctitle.c',
          ],
          'defines': [
            '_DARWIN_USE_64_BIT_INODE=1',
            '_DARWIN_UNLIMITED_SELECT=1',
          ]
        }],
        [ 'OS!="mac"', {
          # Enable on all platforms except OS X. The antique gcc/clang that
          # ships with Xcode emits waaaay too many false positives.
          'cflags': [ '-Wstrict-aliasing' ],
        }],
        [ 'OS=="linux"', {
          'defines': [ '_GNU_SOURCE' ],
          'sources': [
            'libuv/src/unix/linux-core.c',
            'libuv/src/unix/linux-inotify.c',
            'libuv/src/unix/linux-syscalls.c',
            'libuv/src/unix/linux-syscalls.h',
          ],
          'link_settings': {
            'libraries': [ '-ldl', '-lrt' ],
          },
        }],
        [ 'OS=="android"', {
          'sources': [
            'libuv/src/unix/linux-core.c',
            'libuv/src/unix/linux-inotify.c',
            'libuv/src/unix/linux-syscalls.c',
            'libuv/src/unix/linux-syscalls.h',
            'libuv/src/unix/pthread-fixes.c',
            'libuv/src/unix/android-ifaddrs.c'
          ],
          'link_settings': {
            'libraries': [ '-ldl' ],
          },
        }],
        [ 'OS in "freebsd dragonflybsd openbsd netbsd".split()', {
          'link_settings': {
            'libraries': [ '-lkvm' ],
          },
        }],
        [ 'OS in "ios mac freebsd dragonflybsd openbsd netbsd".split()', {
          'sources': [ 'libuv/src/unix/kqueue.c' ],
        }],
      ]
    },
  ]
}
