filter_in_port_tests = executable(
    'filter in port tests',
    'test_filter_port.cpp',
    dependencies : [pipewire_dep],
    include_directories : [include_directory])

test(
    'filter in port tests',
    filter_in_port_tests)

buffer_tests = executable(
    'buffer tests',
    'test_buffer.cpp',
    dependencies : [pipewire_dep],
    include_directories : [include_directory])

test('buffer tests', buffer_tests)

make_props_pod_tests = executable(
    'make_props_pod tests',
    'test_make_props_pod.cpp',
    dependencies : [pipewire_dep],
    include_directories : [include_directory])

test('make_props_pod tests', buffer_tests)
