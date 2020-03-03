################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(smtkCoreModelTesting STATIC helpers.cxx)
smtk_public_headers(smtkCoreModelTesting helpers.h)
target_link_libraries(smtkCoreModelTesting LINK_PRIVATE smtkCore)
smtk_install_library(smtkCoreModelTesting)

################################################################################
# Tools and demos
################################################################################

add_executable(demoReportArrangements demoReportArrangements.cxx)
target_link_libraries(demoReportArrangements smtkCore smtkCoreModelTesting)

################################################################################
# Tests
################################################################################
add_executable(unitManager unitManager.cxx)
target_link_libraries(unitManager smtkCore smtkCoreModelTesting)
add_test(NAME unitManager COMMAND unitManager)

add_executable(unitIterators unitIterators.cxx)
target_link_libraries(unitIterators smtkCore smtkCoreModelTesting)
add_test(NAME unitIterators COMMAND unitIterators)

add_executable(unitTessellation unitTessellation.cxx)
target_link_libraries(unitTessellation smtkCore smtkCoreModelTesting)
add_test(NAME unitTessellation COMMAND unitTessellation)

add_executable(unitEntityRef unitEntityRef.cxx)
target_link_libraries(unitEntityRef smtkCore smtkCoreModelTesting)
add_test(NAME unitEntityRef COMMAND unitEntityRef)

add_executable(unitArrangement unitArrangement.cxx)
target_link_libraries(unitArrangement smtkCore)
add_test(NAME unitArrangement COMMAND unitArrangement)

add_executable(unitExportMeshOperation unitExportMeshOperation.cxx)
target_compile_definitions(unitExportMeshOperation PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(unitExportMeshOperation smtkCore smtkCoreModelTesting
  ${Boost_LIBRARIES})

add_executable(benchmarkModel benchmarkModel.cxx)
target_link_libraries(benchmarkModel smtkCore smtkCoreModelTesting)
#add_test(NAME benchmarkModel COMMAND benchmarkModel)

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR)
  add_test(NAME unitExportMeshOperation
    COMMAND $<TARGET_FILE:unitExportMeshOperation>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json")
endif()

set(additional_libs)

if (SMTK_ENABLE_MESH_SESSION)
  list(APPEND unit_tests_which_require_data TestInterpolateAnnotatedValues.cxx)
  list(APPEND additional_libs smtkMeshSession)
endif ()

if (SMTK_ENABLE_DISCRETE_SESSION)
  list(APPEND unit_tests_which_require_data TestGroupPropertyQuery.cxx)
  list(APPEND additional_libs smtkDiscreteSession)
endif ()

if (SMTK_ENABLE_POLYGON_SESSION)
  list(APPEND unit_tests_which_require_data
    unitEntity.cxx
    unitInstance.cxx
  )
  list(APPEND additional_libs smtkPolygonSession)
endif ()

set(external_libs ${Boost_LIBRARIES})

smtk_unit_tests(
  LABEL "Model"
  # SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkCoreModelTesting ${additional_libs} ${external_libs}
)
