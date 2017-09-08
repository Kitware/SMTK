#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

set (unit_tests
  ImportFromDEFORMOp.cxx
  RevolveOp.cxx
  PartitionBoundariesOp.cxx
)

set (unit_tests_which_require_data)

smtk_unit_tests(
  LABEL "MultiscaleSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkMultiscaleSession smtkMeshSession ${Boost_LIBRARIES})

# All tests for this session require VTK imported as a module from the embedded
# python interpreter.
foreach (test ${unit_tests})
  get_filename_component(tname ${test} NAME_WE)
  set_tests_properties(${tname} PROPERTIES ENVIRONMENT "${smtk_python_test_environment}")
endforeach ()

# All tests for this session require the variable <AFRL_DIR> to be set.
smtk_get_kit_name(kit)
target_compile_definitions(UnitTests_${kit} PRIVATE "AFRL_DIR=\"${AFRL_DIR}\"")
