# Note: We can't put find_package(SMTK REQUIRED) below
#       because SMTK isn't always found when building
#       as part of SMTK.
# ++ 1 ++
cmake_minimum_required(VERSION 3.5)
project(smtk_version)

find_package(SMTK)
add_executable(print_version print_version.cxx)
target_link_libraries(print_version smtkCore)
# -- 1 --

add_test(
  NAME tut-cxx_first_steps
  COMMAND print_version)
