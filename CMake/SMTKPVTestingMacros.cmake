function (_smtk_paraview_add_tests test_function)
  _paraview_add_tests("${test_function}"
    LOAD_PLUGINS smtkPQComponentsPlugin
    PLUGIN_PATHS $<TARGET_FILE_DIR:smtkPQComponentsPlugin
    ${ARGN})
endfunction ()

function (smtk_add_client_tests)
  _paraview_add_tests("smtk_add_client_tests"
    PREFIX "pv"
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
  _paraview_add_tests("smtk_add_client_server_tests"
    PREFIX "pvcs"
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
  _paraview_add_tests("smtk_add_client_server_render_tests"
    PREFIX "pvcrs"
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
