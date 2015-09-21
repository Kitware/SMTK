ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FBiGeom_protos.h
  COMMAND ${CMAKE_COMMAND}
          -DDEF:STRING=MOAB
          -Dinput_file:STRING=${CMAKE_CURRENT_SOURCE_DIR}/FBiGeom.h
          -Doutput_file:STRING=${CMAKE_CURRENT_BINARY_DIR}/FBiGeom_protos.h
          -Dprefix:STRING=FBiGeom
          -P ${MOAB_SOURCE_DIR}/config/ITAPSFortranMangling.cmake
          DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/FBiGeom.h )

set( FBiGEOM_SOURCE FBiGeom_MOAB.hpp FBiGeom_MOAB.cpp )
set( FBiGEOM_INCLUDE_HEADERS FBiGeom.h
                             ${CMAKE_CURRENT_BINARY_DIR}/FBiGeom_protos.h
	                           FBiGeom_f.h )

add_library( FBiGeomMOAB
  ${FBiGEOM_SOURCE}
  ${FBiGEOM_INCLUDE_HEADERS} )

target_link_libraries( FBiGeomMOAB MOAB iMesh )

moab_install_headers(${FBiGEOM_INCLUDE_HEADERS})
install(TARGETS FBiGeomMOAB EXPORT SMTK
                            RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib )
