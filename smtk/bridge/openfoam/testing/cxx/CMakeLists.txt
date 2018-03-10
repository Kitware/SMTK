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
  CreateWindTunnelOp.cxx
  SetWorkingDirectoryOp.cxx
)

set (unit_tests_which_require_data)

smtk_unit_tests(
  LABEL "OpenFOAMSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkOpenFOAMSession smtkMeshSession ${VTK_LIBRARIES} vtkSMTKSourceExt ${Boost_LIBRARIES})

# CreateWindTunnelOp takes a long time to run. Set its timeout to 10 minutes.
set_tests_properties(CreateWindTunnelOp PROPERTIES TIMEOUT 600)

# All tests for this session require VTK imported as a module from the embedded
# python interpreter.
foreach (test ${unit_tests})
  get_filename_component(tname ${test} NAME_WE)
  set_tests_properties(${tname} PROPERTIES ENVIRONMENT "${smtk_python_test_environment}")
endforeach ()
