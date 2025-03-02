project('fr-pmx-ctl-router', 'cpp', default_options : ['cpp_std=c++26'])

add_project_arguments('-Wno-deprecated-declarations', language : 'cpp')
add_project_arguments('-Wno-sign-compare', language : 'cpp')
add_project_arguments('-Wno-deprecated-enum-enum-conversion', language : 'cpp')

pipewire_dep = dependency('libpipewire-0.3')
include_directory = include_directories('include')

subdir('examples')
subdir('test')

