
set(RemoteLibSrcs
  RemusBridgeConnection.cxx
  RemusBridgeConnections.cxx
  RemusRemoteBridge.cxx
  RemusStaticBridgeInfo.cxx
)

set(RemoteLibHeaders
  RemusBridgeConnection.h
  RemusBridgeConnections.h
  RemusRemoteBridge.h
  RemusStaticBridgeInfo.h
)

add_library(SMTKRemote ${RemoteLibSrcs})

# Install the headers
smtk_public_headers(${RemoteLibHeaders})

#set SMTKRemote to publicly link to SMTKCore and privately to Remus
target_link_libraries(SMTKRemote
  LINK_PUBLIC
    SMTKCore
  LINK_PRIVATE
    RemusCommon
    RemusClient
    RemusServer
  )

target_include_directories(SMTKRemote
  PUBLIC
  ${REMUS_INCLUDE_DIRS}
  )

if (SMTK_BUILD_CGM)
  target_include_directories(SMTKRemote PRIVATE ${CGM_INCLUDE_DIRS})
endif()

smtk_export_header(SMTKRemote SMTKRemoteExports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(SMTKRemote PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(SMTKRemote DEPENDS SMTKCore)

# Add a server that accepts arguments for locations to find workers.
add_executable(smtk-model-server
  RemusModelServer.cxx
)

target_link_libraries(smtk-model-server
  SMTKCore
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
  SMTKCore
  SMTKRemote
  RemusCommon
  RemusProto
  RemusClient
  RemusWorker
  ${Boost_LIBRARIES}
)
target_include_directories(smtk-model-worker PRIVATE ${REMUS_INCLUDE_DIRS})
if (SMTK_BUILD_CGM)
  target_link_libraries(smtk-model-worker cgmSMTK)
  target_include_directories(smtk-model-worker PRIVATE ${CGM_INCLUDE_DIRS})
endif()

if (SMTK_BUILD_DISCRETE_BRIDGE)
  target_link_libraries(smtk-model-worker smtkDiscreteBridge)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_remote LINK_PRIVATE RemusCommon)
  target_include_directories(TestBuild_smtk_bridge_remote PRIVATE ${REMUS_INCLUDE_DIRS})

  add_subdirectory(testing)
endif()

if(SMTK_BUILD_PYTHON_WRAPPINGS AND Shiboken_FOUND)
  #extract the headers from the library we built to give them to shiboken

  sbk_wrap_library(SMTKRemote
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
    DEPENDS SMTKCore
  )
  #target_include_directories(SMTKRemotePython PRIVATE ${CGM_INCLUDE_DIRS})
endif()
