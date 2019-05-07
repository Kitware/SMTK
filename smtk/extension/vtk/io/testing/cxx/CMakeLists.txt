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

set(unit_tests
  UnitTestRedirectOutput.cxx
  )

set(unit_tests_which_require_data
  TestImportSTL.cxx
  UnitTestImportExportVTKData.cxx
  UnitTestMeshIOVTK.cxx
  ImportExportMeshWithCellField.cxx
  )

set(unit_test_libs)

smtk_unit_tests(LABEL "vtk/io"
                SOURCES ${unit_tests}
                SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
                LIBRARIES smtkIOVTK smtkCoreModelTesting
                VTK::CommonComputationalGeometry
                VTK::CommonCore
                VTK::CommonDataModel
                VTK::FiltersCore
                VTK::FiltersSources
                VTK::IOXML
                VTK::vtksys
                ${unit_test_libs}
                # TODO: Use Boost:: imported targets
                ${Boost_LIBRARIES})

add_executable(MultiScaleConverter MultiScaleConverter.cxx)
target_link_libraries(MultiScaleConverter
                      LINK_PUBLIC smtkIOVTK
                                  VTK::CommonComputationalGeometry
                                  VTK::CommonCore
                                  VTK::CommonDataModel
                                  VTK::FiltersSources
                                  VTK::IOXML
                                  VTK::vtksys)
target_include_directories(MultiScaleConverter PRIVATE ${MOAB_INCLUDE_DIRS})

add_executable(SurfacePerMaterialExtraction SurfacePerMaterialExtraction.cxx)

target_link_libraries(SurfacePerMaterialExtraction
                      LINK_PUBLIC smtkIOVTK
                                  VTK::CommonComputationalGeometry
                                  VTK::CommonCore
                                  VTK::CommonDataModel
                                  VTK::FiltersSources
                                  VTK::IOXML
                                  VTK::vtksys)
target_include_directories(SurfacePerMaterialExtraction PRIVATE ${MOAB_INCLUDE_DIRS})
