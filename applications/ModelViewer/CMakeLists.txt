cmake_minimum_required(VERSION 3.16)

#-----------------------------------------------------------------------------
# Project name and version
project(ModelViewer VERSION 1.0)

#-----------------------------------------------------------------------------
# Include GNU install directory module to detect where to install
# files on Linux/Unix systems (e.g., lib vs lib64)
include(GNUInstallDirs)

#-----------------------------------------------------------------------------
# Find packages
find_package(smtk REQUIRED)

if (NOT SMTK_ENABLE_VTK_SUPPORT)
  message(FATAL_ERROR
    "This project requires SMTK to be built with VTK support.")
endif()

find_package(VTK REQUIRED COMPONENTS GUISupportQt)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Svg Widgets)

#-----------------------------------------------------------------------------
# Build the executable
add_executable(ModelViewer MACOSX_BUNDLE
  main.cxx
  ModelViewer.cxx
  ModelViewer.ui)

target_link_libraries(ModelViewer
  smtkCore
  smtkMeshSession
  smtkQtExt
  vtkSMTKSourceExt
  vtkSMTKMeshExt
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::InteractionStyle
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::GUISupportQt
  Qt5::Core
  Qt5::Gui
  Qt5::Svg
  Qt5::Widgets
)

vtk_module_autoinit(
    TARGETS ModelViewer
    MODULES VTK::CommonCore
            VTK::CommonDataModel
            VTK::InteractionStyle
            VTK::RenderingCore
            VTK::RenderingOpenGL2
            VTK::GUISupportQt
)

set_target_properties(
  ModelViewer PROPERTIES
    AUTOMOC TRUE
    AUTORCC TRUE
    AUTOUIC TRUE
    )

if (NOT DEFINED CMAKE_INSTALL_NAME_DIR)
  set_target_properties(ModelViewer PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")
endif ()

if (NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH)
  set_target_properties(ModelViewer PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if (APPLE)
  install(TARGETS ModelViewer DESTINATION . COMPONENT Runtime)
elseif(UNIX)
  install(TARGETS ModelViewer COMPONENT Runtime)
endif()
#-----------------------------------------------------------------------------
# Set CPack variables
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "${ModelViewer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ModelViewer_VERSION_MINOR}")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR Kitware)
set(CPACK_SOURCE_TBZ2 OFF)
set(CPACK_SOURCE_TGZ  OFF)
set(CPACK_SOURCE_TZ   OFF)

if(UNIX)
  set(CPACK_GENERATOR "TGZ")
  if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
  endif()
elseif(WIN32)
  set(CPACK_GENERATOR "NSIS")
endif()

#-----------------------------------------------------------------------------
# Set variables for generating the install script
set(bundle ModelViewer)
set(plugin_dest_dir plugins)
set(lib_dest_dir lib)
set(qtconf_dest_dir bin)
set(exe_dest_dir bin)
set(suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(lib_search_dirs)
set(ignore_items)
if(APPLE)
  set(bundle "${bundle}.app")
  set(plugin_dest_dir ${bundle}/Contents/Plugins)
  set(lib_dest_dir ${bundle}/Contents/Libraries)
  set(qtconf_dest_dir ${bundle}/Contents/Resources)
  set(exe_dest_dir ${bundle}/Contents/MacOS)
  if (SMTK_ENABLE_PYTHON_WRAPPING AND Python3_FOUND)
    list(APPEND ignore_items
      # When Python is enabled, the Python executables get added to the list of
      # dependencies to package. We don't need them, so we ignore them
      # explicitly.
      Python
      python${Python3_VERSION_MAJOR}
      python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}
      )
  endif()
elseif(UNIX)
  # For Linux + system Qt5, GET_RUNTIME_DEPENDENCIES has difficulty finding Qt5
  # libraries. We help it out by passing the directories in which these libraries
  # can be found to the algorithm.
  set(targets_to_find Qt5::Gui Qt5::Svg Qt5::Widgets)

  # For VTK-only builds (no ParaView), VTK is often built with kits enabled.
  # VTK's Filters kit brings in VTK::FiltersVerdict, which depends on
  # VTK::verdict. For some reason, GET_RUNTIME_DEPENDENCIES has difficulty
  # finding VTK::verdict, so we add it here too.
  list(APPEND targets_to_find VTK::verdict)

  foreach(target IN LISTS targets_to_find)
    get_target_property(loc ${target} LOCATION)
    get_filename_component(real_loc ${loc} REALPATH)
    get_filename_component(dir ${real_loc} DIRECTORY)
    list(APPEND lib_search_dirs ${dir})
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Add the plugins needed by the Qt frameworks used by the application
set(qt5_version "${Qt5Core_VERSION_MAJOR}.${Qt5Core_VERSION_MINOR}")

set(qt_plugins
  Qt5::QSvgIconPlugin
  Qt5::QSvgPlugin
  )

if (APPLE)
  list(APPEND qt_plugins
    Qt5::QCocoaIntegrationPlugin
    Qt5::QCocoaPrinterSupportPlugin
    )

  if (NOT qt_version VERSION_LESS "5.10")
    list(APPEND qt5_plugins
      Qt5::QMacStylePlugin
      )
  endif ()
elseif (UNIX)
  list(APPEND qt_plugins
    Qt5::QXcbIntegrationPlugin
    Qt5::QComposePlatformInputContextPlugin
    Qt5::QXcbGlxIntegrationPlugin
    )
endif ()

foreach(plugin IN LISTS qt_plugins)
  # Qt plugins are required to have the same directory structure as
  # in their install tree, so we replicate it here.
  get_target_property(loc ${plugin} LOCATION)
  get_filename_component(dir ${loc} DIRECTORY)
  get_filename_component(last_dir ${dir} NAME)
  install(FILES "${loc}" DESTINATION "${plugin_dest_dir}/${last_dir}" COMPONENT Runtime)
endforeach()

#-----------------------------------------------------------------------------
# For UNIX, some libraries may be system installed, but we still
# want them bundled. For example, LibXml2 is frequently installed as
# a system library. If this setup has system libraries that we want,
# bundled, the call to GET_RUNTIME_DEPENDENCIES below will omit them
# from them package. We therefore force their inclusion if they are
# available as system libraries.
if (UNIX AND NOT APPLE)
  set(required_libs)
  find_package(ZLIB QUIET)
  find_package(LibXml2 QUIET)

  if (ZLIB_FOUND)
    list(APPEND required_libs ZLIB::ZLIB)
  endif()
  if (LibXml2_FOUND)
    list(APPEND required_libs LibXml2::LibXml2)
  endif()
  foreach(lib IN LISTS required_libs)
    get_target_property(loc ${lib} LOCATION)
    get_filename_component(real_loc ${loc} REALPATH)
    get_filename_component(real_loc_wle ${real_loc} NAME_WLE)
    get_filename_component(real_loc_wle ${real_loc_wle} NAME_WLE)
    install(FILES "${real_loc}" DESTINATION ${lib_dest_dir} RENAME ${real_loc_wle} COMPONENT Runtime)
  endforeach()
endif()

#-----------------------------------------------------------------------------
# install a qt.conf file
if (APPLE)
  install(CODE "
      file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
      " COMPONENT Runtime)
elseif (UNIX)
  # Linux builds need the path to the plugins directory, so we add it here.
  install(CODE "
      file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\nPrefix = ..\")
      " COMPONENT Runtime)
endif()

#-----------------------------------------------------------------------------
# Generate the packaging script
install(CODE "set(bundle \"${bundle}\")" COMPONENT Runtime)
install(CODE "set(plugin_dest_dir \"${plugin_dest_dir}\")" COMPONENT Runtime)
install(CODE "set(lib_dest_dir \"${lib_dest_dir}\")" COMPONENT Runtime)
install(CODE "set(exe_dest_dir \"${exe_dest_dir}\")" COMPONENT Runtime)
install(CODE "set(suffix \"${CMAKE_SHARED_LIBRARY_SUFFIX}\")" COMPONENT Runtime)
install(CODE "set(ignore_items \"${ignore_items}\")" COMPONENT Runtime)
install(CODE "set(lib_search_dirs \"${lib_search_dirs}\")" COMPONENT Runtime)
install(CODE
  "# Construct a list of the application's plugins
   file(GLOB_RECURSE qt_plugins \"\${CMAKE_INSTALL_PREFIX}/\${plugin_dest_dir}/*/*\${suffix}\")

   # Construct a list of libraries already in the application bundle
   file(GLOB_RECURSE required_libs \"\${CMAKE_INSTALL_PREFIX}/\${lib_dest_dir}/*\${suffix}\")

   # Construct a regex for libraries that we should package, even if they are
   # considered system libraries (e.g., \"^.*(gfortran|quadmath).*\$\").
   set(post_include_regex \"\")

   # Construct a list of the application's runtime dependencies
   file(GET_RUNTIME_DEPENDENCIES
          RESOLVED_DEPENDENCIES_VAR resolved_deps
          UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
          CONFLICTING_DEPENDENCIES_PREFIX conflicting_deps
          POST_EXCLUDE_REGEXES \"^/(System|lib|usr/lib)\"
          EXECUTABLES \${CMAKE_INSTALL_PREFIX}/\${exe_dest_dir}/ModelViewer
          LIBRARIES \${qt_plugins} \${required_libs}
          DIRECTORIES \${lib_search_dirs}
          )

   # Conflicting dependencies are dependencies that are found in multiple
   # places. If a dependency in this list matches our post_include_regex
   # and is not in our ignore_items list, use the first instance of the
   # dependency found.
   set(conflicting_deps)
   foreach (conflicting_dep IN LISTS conflicting_deps_FILENAMES)
     if (NOT post_include_regex)
       continue ()
     endif ()
     string(REGEX MATCH \"\${post_include_regex}\" match \"\${conflicting_dep}\")
     if (match)
       set(add_to_list ON)
         if (ignore_items)
           if (\${conflicting_dep} NOT IN_LIST ignore_items)
             set(add_to_list OFF)
           endif()
         endif()
       if (add_to_list)
         list(GET conflicting_deps_\${conflicting_dep} 0 dep)
         list(APPEND conflicting_deps \${dep})
       endif()
     endif()
   endforeach()

   include(BundleUtilities)

   # Copy each resolved dependency into the application bundle
   foreach (lib IN LISTS resolved_deps conflicting_deps required_libs)
      get_filename_component(libname \${lib} NAME)

      # Copy the library into the bundle
      execute_process(COMMAND \${CMAKE_COMMAND} -E
                      copy \${lib} \${CMAKE_INSTALL_PREFIX}/\${lib_dest_dir}/\${libname})

      if (UNIX AND NOT APPLE)
        # Change its permissions to allow us to modify its rpath
        execute_process(COMMAND chmod u+w \"\${CMAKE_INSTALL_PREFIX}/\${lib_dest_dir}/\${libname}\")

        # Modify its rpath so it only attempts to resolve dependencies within
        # the bundle
        execute_process(COMMAND
          chrpath --replace \$ORIGIN \"\${CMAKE_INSTALL_PREFIX}/\${lib_dest_dir}/\${libname}\")
      endif()
   endforeach()

   # Linux requires that the bundle's plugins also have a relative rpath set
   if(UNIX AND NOT APPLE)
     foreach (plugin IN LISTS qt_plugins)
       execute_process(COMMAND
         chrpath --replace \"\$ORIGIN:\$ORIGIN/../lib\" \"\${plugin}\"
         RESULT_VARIABLE ignore)
     endforeach()
   endif()

   # Construct a list of directories inside the bundle where the application
   # should look to resolve its dependencies
   set(dirs \${CMAKE_INSTALL_PREFIX}/\${lib_dest_dir})
   list(APPEND dirs \${CMAKE_INSTALL_PREFIX}/\${CMAKE_INSTALL_LIBDIR})

   # Add the bundle's library directory to the executable's rpath list
   if (APPLE)
     set(rpath \"@executable_path/../Libraries/\")
     execute_process(COMMAND
       ${CMAKE_INSTALL_NAME_TOOL} -add_rpath \"\${rpath}\"
       \${CMAKE_INSTALL_PREFIX}/\${exe_dest_dir}/ModelViewer)
   elseif(UNIX)
     set(rpath \"\$ORIGIN/../lib\")
        execute_process(COMMAND
          chrpath --replace \"\${rpath}\" \${CMAKE_INSTALL_PREFIX}/\${exe_dest_dir}/ModelViewer)
   endif()

   # By default, CPack on Apple puts all libraries and Frameworks in the Frameworks
   # directory, and on Linux it puts all libraries in the bin directory. This behavior
   # can be modified by defining the following function, which CMake looks for when
   # fixup_bundle is run.
   function(gp_item_default_embedded_path_override item default_embedded_path_var)

     # By default, embed items as set by gp_item_default_embedded_path
     # (which puts things in Framework):
     set(path \"\${\${default_embedded_path_var}}\")

     # If we're on APPLE and it's a library, put it in Libraries
     if(APPLE AND item MATCHES \"\${suffix}\$\")
       set(path \"@executable_path/../Libraries\")
     endif()

     # If we're on Linux and it's a library, put it in lib
     if(UNIX AND NOT APPLE)
       get_filename_component(item_name \${item} NAME)
       if (item_name MATCHES \"^lib.*\${suffix}.*\")
         set(path \"@executable_path/../lib\")
       endif()
     endif()

     set(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)

   endfunction()

   # Becasue we use the above custom function to decide where libraries go, we
   # must also provide a custom function that verifies if an item is in the
   # bundle.
   function(gp_resolved_file_type_override resolved_file type_var)

     # For CPack on Linux, the default behavior of this method involves
     # checking if an item's path is the same as the executable. Since we
     # have split libraries into their own subdirectory, we must accommodate
     # the new directory layout here.
     if (UNIX AND NOT APPLE AND \${type_var} STREQUAL \"other\")
       get_filename_component(dir \${resolved_file} DIRECTORY)
       if (\"\${dir}\" MATCHES \"^\${CMAKE_INSTALL_PREFIX}*\")
         set(\${type_var} \"local\" PARENT_SCOPE)
       endif()
     endif()

   endfunction()

   # Change the access rights for objects copied into the bundle, so the fixup
   # logic can correctly fix them
   set(BU_CHMOD_BUNDLE_ITEMS TRUE)

   # Execute fixup_bundle on the application (1) with additional plugins (2) using
   # the listed directories (3) to find dependencies. Also, ignore unwanted
   # dependencies (4) that may have been incorrectly gathered during the
   # dependency search.
   fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/\${exe_dest_dir}/ModelViewer\"
                \"\${qt_plugins}\" \"\${dirs}\" IGNORE_ITEM \"\${ignore_items}\")
  "
  COMPONENT Runtime)

include(CPack)
