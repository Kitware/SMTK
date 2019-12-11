function (_smtk_paraview_add_tests test_function)
  _paraview_add_tests("${test_function}"
    LOAD_PLUGINS smtkPQComponentsPlugin
    PLUGIN_PATHS $<TARGET_FILE_DIR:smtkPQComponentsPlugin
    ${ARGN})
endfunction ()

function (_smtk_sanitizer_env variable)
  set(smtk_paraview_test_environment)
  if (SMTK_ENABLE_SANITIZER)
    set(preload_libraries)
    if (SMTK_SANITIZER MATCHES "address")
      find_library(SMTK_ASAN_LIBRARY NAMES libasan.so.5 DOC "ASan library")
      mark_as_advanced(SMTK_ASAN_LIBRARY)

      list(APPEND preload_libraries
        "${SMTK_ASAN_LIBRARY}")
    endif ()

    if (preload_libraries)
      if (UNIX AND NOT APPLE)
        list(APPEND smtk_paraview_test_environment
          "LD_PRELOAD=${preload_libraries}")
      endif ()
    endif ()
  endif ()

  set("${variable}"
    "${smtk_paraview_test_environment}"
    PARENT_SCOPE)
endfunction ()

function (smtk_add_client_tests)
  _smtk_sanitizer_env(sanitizer_env)

  _paraview_add_tests("smtk_add_client_tests"
    PREFIX "pv"
    ENVIRONMENT
      ${sanitizer_env}
    _DISABLE_SUFFIX "_DISABLE_C"
    _COMMAND_PATTERN
      --client __paraview_client__
        --enable-bt
        __paraview_args__
        __paraview_script__
        __paraview_client_args__
        -dr
        --exit
    ${ARGN})
endfunction ()

function (smtk_add_client_server_tests)
  _smtk_sanitizer_env(sanitizer_env)

  _paraview_add_tests("smtk_add_client_server_tests"
    PREFIX "pvcs"
    ENVIRONMENT
      ${sanitizer_env}
    _DISABLE_SUFFIX "_DISABLE_CS"
    _COMMAND_PATTERN
      --server "$<TARGET_FILE:ParaView::pvserver>"
        --enable-bt
        __paraview_args__
      --client __paraview_client__
        --enable-bt
        __paraview_args__
        __paraview_script__
        __paraview_client_args__
        -dr
        --exit
    ${ARGN})
endfunction ()

function (smtk_add_client_server_render_tests)
  _smtk_sanitizer_env(sanitizer_env)

  _paraview_add_tests("smtk_add_client_server_render_tests"
    PREFIX "pvcrs"
    ENVIRONMENT
      ${sanitizer_env}
    _DISABLE_SUFFIX "_DISABLE_CRS"
    _COMMAND_PATTERN
      --data-server "$<TARGET_FILE:ParaView::pvdataserver>"
        --enable-bt
        __paraview_args__
      --render-server "$<TARGET_FILE:ParaView::pvrenderserver>"
        --enable-bt
        __paraview_args__
      --client __paraview_client__
        --enable-bt
        __paraview_args__
        __paraview_script__
        __paraview_client_args__
        -dr
        --exit
    ${ARGN})
endfunction ()
