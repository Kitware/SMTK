include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set( SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P} )

configure_file(iBase_f.h.in "${CMAKE_CURRENT_BINARY_DIR}/iBase_f.h" @ONLY)

set( BASE_INCLUDE_HEADERS iBase.h ${CMAKE_CURRENT_BINARY_DIR}/iBase_f.h)

if(ENABLE_IMESH )
  add_subdirectory(imesh)
endif()

if(ENABLE_IGEOM )
  add_subdirectory(igeom)
endif()

install(FILES ${BASE_INCLUDE_HEADERS} DESTINATION include )
