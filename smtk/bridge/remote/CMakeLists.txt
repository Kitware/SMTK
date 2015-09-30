set(RemoteLibSrcs
  RemusConnection.cxx
  RemusConnections.cxx
  Session.cxx
  RemusStaticSessionInfo.cxx
)

set(RemoteLibHeaders
  RemusConnection.h
  RemusConnections.h
  Session.h
  RemusStaticSessionInfo.h
)

add_library(smtkRemoteSession ${RemoteLibSrcs})

# Install the headers
smtk_public_headers(${RemoteLibHeaders})

#set smtkRemoteSession to publicly link to smtkCore and privately to Remus
target_link_libraries(smtkRemoteSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    RemusCommon
    RemusClient
    RemusServer
  )

target_include_directories(smtkRemoteSession
  PUBLIC
  ${REMUS_INCLUDE_DIRS}
  )

if (SMTK_ENABLE_CGM_SESSION)
  target_include_directories(smtkRemoteSession PRIVATE ${CGM_INCLUDE_DIRS})
endif()

smtk_export_header(smtkRemoteSession Exports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkRemoteSession PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkRemoteSession)

# Add a server that accepts arguments for locations to find workers.
add_executable(smtk-model-server
  smtk-model-server.cxx
)

target_link_libraries(smtk-model-server
  smtkCore
  RemusCommon
  RemusServer
  ${Boost_LIBRARIES}
)
target_include_directories(smtk-model-server PRIVATE ${REMUS_INCLUDE_DIRS})

# Add a worker that links to each modeling kernel we bridge
add_executable(smtk-model-worker
  smtk-model-worker.cxx
  RemusRPCWorker.cxx
)

target_link_libraries(smtk-model-worker
  smtkCore
  smtkRemoteSession
  RemusCommon
  RemusProto
  RemusClient
  RemusWorker
  ${Boost_LIBRARIES}
)
target_include_directories(smtk-model-worker PRIVATE ${REMUS_INCLUDE_DIRS})
if (SMTK_ENABLE_CGM_SESSION)
  target_link_libraries(smtk-model-worker smtkCGMSession)
  target_include_directories(smtk-model-worker PRIVATE ${CGM_INCLUDE_DIRS})
endif()

if (SMTK_ENABLE_DISCRETE_SESSION)
  target_link_libraries(smtk-model-worker smtkDiscreteSession)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_remote LINK_PRIVATE RemusCommon)
  target_include_directories(TestBuild_smtk_bridge_remote PRIVATE ${REMUS_INCLUDE_DIRS})

  add_subdirectory(testing)
endif()

#install both the server and worker
install(TARGETS smtk-model-server smtk-model-worker
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from the library we built to give them to shiboken

  sbk_wrap_library(smtkRemoteSession
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/bridge
      ${SMTK_SOURCE_DIR}/smtk/bridge/remote
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${SMTK_SOURCE_DIR}/smtk/bridge/remote/typesystem.xml
    HEADERS ${RemoteLibHeaders}
    DEPENDS smtkCore
  )
  #target_include_directories(smtkRemoteSessionPython PRIVATE ${CGM_INCLUDE_DIRS})
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()
