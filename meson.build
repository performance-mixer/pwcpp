project('fr-pmx-ctl-router', 'cpp', default_options : ['cpp_std=c++26'])
pipewire_dep = dependency('libpipewire-0.3')

include_directory = include_directories('include')

spa_debug_input = executable(
  'spa_debug_input',
  'examples/spa_debug_input.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

spa_debug_input = executable(
  'property',
  'examples/property.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

filter_app_tests = executable(
  'filter app tests', 
  'test/test_filter_app.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

test(
  'filter app tests',
  filter_app_tests)

filter_app_builder_tests = executable(
  'filter app builder tests',
  'test/test_filter_app_builder.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

test(
  'filter app builder tests',
  filter_app_builder_tests)

filter_in_port_tests = executable(
  'filter in port tests',
  'test/test_filter_port.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

test(
  'filter in port tests',
  filter_in_port_tests)

buffer_tests = executable(
  'buffer tests',
  'test/test_buffer.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

test('buffer tests', buffer_tests)

make_props_pod_tests = executable(
  'make_props_pod tests',
  'test/test_make_props_pod.cpp',
  dependencies: [pipewire_dep],
  include_directories: [include_directory])

test('make_props_pod tests', buffer_tests)
