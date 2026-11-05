add_subdirectory(experimental)

# The staging tree is populated via a directory symlink created at configure
# time in src/CMakeLists.txt; no per-file copy is needed here.
add_custom_target(pymetkit)
add_dependencies(pymetkit pymetkit-experimental)
