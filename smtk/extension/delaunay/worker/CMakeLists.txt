set(worker_library_srcs
  detail/fromRemus.cxx
  DelaunayMeshWorker.cxx
  )

set(worker_exec_srcs
  DelaunayMeshWorkerMain.cxx
  )

add_library(DelaunayWorker STATIC ${worker_library_srcs})

target_link_libraries(DelaunayWorker
                      LINK_PUBLIC
                      RemusWorker
                      smtkCore
                      smtkDelaunayExt
                      DelaunayShape
                      DelaunayMesh
                      DelaunayMisc
                      DelaunayValidation
                      DelaunayDiscretization
                      )

target_include_directories(DelaunayWorker
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           )

set_target_properties(DelaunayWorker
                      PROPERTIES
                      POSITION_INDEPENDENT_CODE ON
  )
			 
add_executable(DelaunayMeshWorker ${worker_exec_srcs} )
target_link_libraries(DelaunayMeshWorker
                      LINK_PRIVATE
                      DelaunayWorker
                      )

if(WIN32)
  #on windows you want to tell boost to only link to the libs it needs
  target_compile_definitions(DelaunayWorker PRIVATE BOOST_ALL_NO_LIB)
  target_compile_definitions(DelaunayMeshWorker PRIVATE BOOST_ALL_NO_LIB)
endif()


#setup the workers to run in the build directory
#########################################################################
file(COPY
    "${CMAKE_CURRENT_SOURCE_DIR}/DelaunayMeshingDefs.sbt"
    "${CMAKE_CURRENT_SOURCE_DIR}/DelaunayMesh.rw"
    DESTINATION
    ${CMAKE_BINARY_DIR}/bin
    )

#setup the workers to be installed and run from the installed directory
#########################################################################
set_target_properties(
  DelaunayMeshWorker PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS DelaunayMeshWorker
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/DelaunayMeshingDefs.sbt"
        "${CMAKE_CURRENT_SOURCE_DIR}/DelaunayMesh.rw"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
