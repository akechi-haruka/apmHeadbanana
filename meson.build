project('apmHeadbananaVolume', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c17'])

add_project_arguments(
    '-DWIN32_LEAN_AND_MEAN',
    '-Wno-unused-parameter',
    '-Wno-unknown-pragmas',
    '-s',
    '-g0',
    language: 'cpp',
)

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_MESON_LIBRARY']

shared_library('apmHeadbananaVolume', 'main.cpp', 'dprintf.h', 'dprintf.cpp',
               c_args : lib_args,
               vs_module_defs : 'exports.def',
)