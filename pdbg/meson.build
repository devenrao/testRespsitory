cxx = meson.get_compiler('cpp')

pdbg_deps = [
    cxx.find_library('pdbg'),
    cxx.find_library('fdt'),
    cxx.find_library('ekb'),
    cxx.find_library('dt-api')
]
executable(
    'proc_path',
    'print_proc_path.cpp',
    dependencies: pdbg_deps,
    install: true,
)
