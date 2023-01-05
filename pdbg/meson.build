cxx = meson.get_compiler('cpp')

pdbg_deps = [
    cxx.find_library('pdbg'),
    cxx.find_library('fdt'),
    cxx.find_library('ekb'),
]

executable(
    'backend_switch',
    'backend_switch.cpp',
    dependencies: pdbg_deps,
    install: true,
)
executable(
    'print_proc_type',
    'print_proc_type.cpp',
    dependencies: pdbg_deps,
    install: true,
)    
