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
    RemusCommon
  LINK_PRIVATE
    RemusClient
    RemusServer
  )

if (SMTK_ENABLE_CGM_SESSION)
  target_include_directories(smtkRemoteSession PRIVATE ${CGM_INCLUDE_DIRS})
endif()

smtk_export_header(smtkRemoteSession Exports.h)

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

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()
