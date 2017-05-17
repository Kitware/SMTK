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
  UnitTestAllocator.cxx
  UnitTestCellTypes.cxx
  UnitTestCollection.cxx
  UnitTestBufferedCellAllocator.cxx
  UnitTestIncrementalAllocator.cxx
  UnitTestManager.cxx
  UnitTestModelToMesh3D.cxx
  UnitTestQueryTypes.cxx
  UnitTestReadWriteHandles.cxx
  UnitTestTypeSet.cxx
)

set(unit_tests_which_require_data
  UnitTestAddFileToCollection.cxx
  UnitTestCellSet.cxx
  UnitTestCreateMesh.cxx
  UnitTestCellField.cxx
  UnitTestElevate.cxx
  UnitTestExportMesh2DM.cxx
  UnitTestExportMesh3DM.cxx
  UnitTestExtractMeshConstants.cxx
  UnitTestExtractTessellation.cxx
  UnitTestExtractTessellationOfModel.cxx

  UnitTestLoadMesh.cxx
  UnitTestMergeContactPoints.cxx
  UnitTestMeshSet.cxx
  UnitTestModelToMesh2D.cxx
  UnitTestPointConnectivity.cxx
  UnitTestPointField.cxx
  UnitTestPointLocator.cxx
  UnitTestPointSet.cxx
  UnitTestReadWriteMeshJSON.cxx
  UnitTestReclassifyEdges.cxx
  UnitTestRemoveMeshes.cxx
  UnitTestTypeSetFromData.cxx
  UnitTestWriteMesh.cxx
)

set(extra_libs)
if(SMTK_ENABLE_DISCRETE_SESSION)
  list(APPEND unit_tests_which_require_data
       UnitTestExtractOrderedTessellation.cxx
       )
  list(APPEND extra_libs
       smtkDiscreteSession
       )
endif()

smtk_unit_tests(
  LABEL "Mesh"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkCoreModelTesting ${extra_libs} ${Boost_LIBRARIES}
)

add_executable(UnitTestInterpolateMesh UnitTestInterpolateMesh.cxx)
target_compile_definitions(UnitTestInterpolateMesh PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(UnitTestInterpolateMesh smtkCore ${Boost_LIBRARIES})

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  add_test(NAME UnitTestInterpolateMeshToCells
    COMMAND $<TARGET_FILE:UnitTestInterpolateMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json")
  set_tests_properties(UnitTestInterpolateMeshToCells PROPERTIES LABELS "Mesh")
  add_test(NAME UnitTestInterpolateMeshToPoints
    COMMAND $<TARGET_FILE:UnitTestInterpolateMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 0)
  set_tests_properties(UnitTestInterpolateMeshToPoints PROPERTIES LABELS "Mesh")
  add_test(NAME UnitTestInterpolateMeshToCellsCSV
    COMMAND $<TARGET_FILE:UnitTestInterpolateMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 0 1)
  set_tests_properties(UnitTestInterpolateMeshToCellsCSV PROPERTIES LABELS "Mesh")
  add_test(NAME UnitTestInterpolateMeshToPointsCSV
    COMMAND $<TARGET_FILE:UnitTestInterpolateMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 1 1)
  set_tests_properties(UnitTestInterpolateMeshToPointsCSV PROPERTIES LABELS "Mesh")
endif()
