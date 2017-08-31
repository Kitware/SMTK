#Build the Delaunay worker test executable, this integrates
#a remus client and server and verifies that the mesh worker behaves properly
#########################################################################

if(SMTK_ENABLE_POLYGON_SESSION AND SMTK_DATA_DIR)

  find_package(Boost 1.55.0
    COMPONENTS thread filesystem system REQUIRED)

  add_executable(UnitTestDelaunayMeshWorker
                 UnitTestDelaunayMeshWorker.cxx)

  target_link_libraries(UnitTestDelaunayMeshWorker
                        LINK_PRIVATE
                        DelaunayWorker
                        smtkCore
                        smtkPolygonSession
                        smtkRemusExt
                        RemusClient
                        RemusServer
                        ${Boost_LIBRARIES}
                        ${CMAKE_THREAD_LIBS_INIT}
                        )

  add_test(NAME UnitTestDelaunayMeshWorker
           COMMAND $<TARGET_FILE:UnitTestDelaunayMeshWorker>
                   "${SMTK_DATA_DIR}/mesh/2d/boxWithHole.smtk")
  set_tests_properties(UnitTestDelaunayMeshWorker PROPERTIES
                       LABELS "ExtDelaunay")

  add_executable(TestDelaunayMeshWorker
                 TestDelaunayMeshWorker.cxx)

  target_link_libraries(TestDelaunayMeshWorker
                        LINK_PRIVATE
                        DelaunayWorker
                        smtkCore
                        smtkPolygonSession
                        RemusClient
                        RemusServer
                        ${Boost_LIBRARIES}
                        ${CMAKE_THREAD_LIBS_INIT}
                        )

  add_test(NAME TestDelaunayMeshWorker
           COMMAND $<TARGET_FILE:TestDelaunayMeshWorker>
                   "${SMTK_DATA_DIR}/mesh/2d/boxWithHole.smtk")
  set_tests_properties(TestDelaunayMeshWorker PROPERTIES
                       LABELS "ExtDelaunay")
endif()
