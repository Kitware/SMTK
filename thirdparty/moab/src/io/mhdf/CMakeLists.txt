include_directories( ${HDF5_INCLUDE_DIR} )

set ( mhdf_LIB_SRCS
        include/mhdf.h
        src/adjacency.c
        src/connectivity.c
        src/file-desc.c
        src/file-handle.h  src/file-handle.c
        src/file.c
        src/names-and-paths.h
        src/nodes.c
        src/sets.c
        src/status.h src/status.c
        src/tags.c
        src/util.h src/util.c
)

add_library( mhdf OBJECT ${mhdf_LIB_SRCS} )
if (BUILD_SHARED_LIBS)
  set(shared_define "-DIS_BUILDING_MB_SHARED")
  set_target_properties(mhdf PROPERTIES POSITION_INDEPENDENT_CODE True)
endif ()
set_target_properties( mhdf
                       PROPERTIES COMPILE_FLAGS "${shared_define}")

if (FALSE)

add_executable( h5minfo include/mhdf.h example/info.c $<TARGET_OBJECTS:mhdf> )
target_link_libraries( h5minfo ${HDF5_C_LIBRARIES} )
install(TARGETS h5minfo RUNTIME DESTINATION bin
                        LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib )

add_executable( h5mvalidate example/validate.c $<TARGET_OBJECTS:mhdf> )
install(TARGETS h5mvalidate RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib )
target_link_libraries( h5mvalidate ${HDF5_C_LIBRARIES} )

endif ()
