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

set (unit_tests)

set (unit_tests_which_require_data
  UnitTestMeshSessionReadWrite.cxx)

set(external_libs ${Boost_LIBRARIES})

if (SMTK_ENABLE_VTK_SUPPORT)
set_source_files_properties(UnitTestMeshSessionTopology.cxx
  PROPERTIES COMPILE_FLAGS -DSMTK_ENABLE_VTK_SUPPORT)
list (APPEND external_libs ${VTK_LIBRARIES} vtkSMTKSourceExt smtkIOVTK)
endif()

smtk_unit_tests(
  LABEL "MeshSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkMeshSession smtkCoreModelTesting ${Boost_LIBRARIES}
)
