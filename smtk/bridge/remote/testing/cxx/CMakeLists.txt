add_executable(unitRemusBridgeConnection
  unitRemusBridgeConnection.cxx
)
target_link_libraries(unitRemusBridgeConnection
  SMTKRemote
  SMTKCore
  RemusCommon
  RemusClient
)
if (SMTK_BUILD_CGM)
  target_link_libraries(unitRemusBridgeConnection cgmSMTK)
endif()
if (SMTK_ENABLE_TESTING AND SMTK_DATA_DIR AND SMTK_BUILD_CGM)
  add_test(
    NAME unitRemusBridgeConnection
    COMMAND
      unitRemusBridgeConnection
      ${SMTK_DATA_DIR}/cgm/pyramid.brep
      "smtk::model[cgm{OpenCascade}]"
  )
endif()
