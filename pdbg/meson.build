cxx = meson.get_compiler('cpp')

backend_switch_deps = [
    cxx.find_library('pdbg'),
    cxx.find_library('fdt'),
    cxx.find_library('ekb'),
]

executable(
    'backend_switch',
    'backend_switch.cpp',
    dependencies: backend_switch_deps,
    install: true,
)
