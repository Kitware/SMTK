# ++ 1 ++
cmake_minimum_required(VERSION 3.12)
project(smtk_version)

find_package(smtk REQUIRED)
add_executable(print_version print_version.cxx)
target_link_libraries(print_version smtkCore)
# -- 1 --

include(CTest)
enable_testing()

add_test(
  NAME tut-cxx_first_steps
  COMMAND print_version)
