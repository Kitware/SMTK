add_executable(integrationRemoteSession
  integrationRemoteSession.cxx
)
target_link_libraries(integrationRemoteSession
  smtkRemoteSession
  smtkCore
  RemusCommon
  RemusClient
)
if (SMTK_BUILD_CGM)
  target_link_libraries(integrationRemoteSession smtkCGMSession)
endif()
if (SMTK_ENABLE_TESTING AND SMTK_DATA_DIR AND SMTK_BUILD_CGM)
  add_test(
    NAME unitGenerateRemoteModelWorker
    COMMAND
      smtk-model-worker -generate
      -kernel=cgm -engine=OpenCascade
      -rwfile=${SMTK_BINARY_DIR}/Testing/cgm-occ.rw
      -root=${SMTK_BINARY_DIR}/Testing
      -site=ctest
  )
  add_test(
    NAME integrationRemoteSession
    COMMAND
      integrationRemoteSession
      ${SMTK_DATA_DIR}/cgm/pyramid.brep
      "smtk::model[cgm{OpenCascade}@ctest]"
      "local" 50505
      ${SMTK_BINARY_DIR}/Testing
  )
  set_tests_properties(
    integrationRemoteSession
    PROPERTIES
      DEPENDS unitGenerateRemoteModelWorker
  )
endif()
