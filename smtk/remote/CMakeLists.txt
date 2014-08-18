include_directories(${REMUS_INCLUDE_DIRS})

set(RemoteLibSrcs
  RemusBridgeConnection.cxx
  RemusBridgeConnections.cxx
  RemusRemoteBridge.cxx
)

set(RemoteLibHeaders
  RemusBridgeConnection.h
  RemusBridgeConnections.h
  RemusRemoteBridge.h
)

# Install the headers
smtk_public_headers(${RemoteLibHeaders})
if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_remote
    LINK_PRIVATE
      RemusCommon
  )
endif()

# Add a server that accepts arguments for locations to find workers.
add_executable(smtk-model-server
  RemusModelServer.cxx
)
target_link_libraries(smtk-model-server
  SMTKCore
  RemusCommon
  RemusServer
)

# Add a worker that links to each modeling kernel we bridge
add_executable(smtk-remote-model
  smtk-remote-model.cxx
  RemusRPCWorker.cxx
)
target_link_libraries(smtk-remote-model
  SMTKCore
  SMTKRemote
  RemusCommon
  RemusProto
  RemusClient
  RemusWorker
)
if (SMTK_BUILD_CGM)
  target_link_libraries(smtk-remote-model cgmSMTK)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
