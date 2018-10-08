# ++ 1 ++
cmake_minimum_required(VERSION 3.5)
project(smtk_version)

find_package(smtk)
add_executable(print_version print_version.cxx)
target_link_libraries(print_version smtkCore)
# -- 1 --
