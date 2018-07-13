set(workflowSrcs
  OperationFilterSort.cxx
  json/jsonOperationFilterSort.cxx
)

set(jsonWorkflowHdrs
  json/jsonOperationFilterSort.h
)
set(workflowHdrs
  OperationFilterSort.h
  ${jsonWorkflowHdrs}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  #add_subdirectory(pybind11)
endif()

# Install the headers
smtk_public_headers(${workflowHdrs})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  target_link_libraries(TestBuild_smtk_workflow
    PUBLIC nlohmann_json)
endif()
