set(jobClasses
  DatabaseQueue
  DefinitionInstances
  Definition
  Job
  Queue
  QueueInstances
  LogParser
  Manager
  Registrar
  Stage
  State
  Status
)

set(jobSrcs)
set(jobHeaders)

foreach(class ${jobClasses})
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.h")
    list(APPEND jobHeaders "${class}.h")
  endif()
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.cxx")
    list(APPEND jobSrcs "${class}.cxx")
  endif()
endforeach()

set(jobOperators
  AddJobToQueue
  CancelJob
  JobUpdated
  ScheduleJob
)

foreach (operator ${jobOperators})
  list(APPEND jobHeaders operators/${operator}.h)
  list(APPEND jobSrcs operators/${operator}.cxx)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt")
    smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
      HEADER_OUTPUT headerName)
    list(APPEND _jobDependencies ${headerName})
  endif()
endforeach()
add_custom_target(jobGenHeaders
  DEPENDS
    ${_jobDependencies}
    ${smtkJobDependencies}
)
# set(jobDependencies ${_jobDependencies} PARENT_SCOPE)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${jobHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
