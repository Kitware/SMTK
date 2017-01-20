set( MOAB_LOC_DSCR_SRCS ElemEvaluator.cpp
                        LinearHex.cpp
                        LinearQuad.cpp
                        LinearTet.cpp
                        LinearTri.cpp
                        QuadraticHex.cpp )

set( MOAB_LOCALDSRC_Install_Headers moab/ElemEvaluator.hpp
                                    moab/LinearHex.hpp
                                    moab/LinearQuad.hpp
	                                  moab/LinearTet.hpp
                                    moab/LinearTri.hpp
                                    moab/QuadraticHex.hpp )

add_library( moab_loc_discr OBJECT ${MOAB_LOC_DSCR_SRCS}
                                   ${MOAB_LOCALDSRC_Install_Headers} )

# Set position-independent flag so that the object files are embedded properly.
set_target_properties(moab_loc_discr PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if (BUILD_SHARED_LIBS)
  set(shared_define "-DIS_BUILDING_MB_SHARED")
endif ()
set_target_properties(moab_loc_discr PROPERTIES POSITION_INDEPENDENT_CODE True)
set_target_properties( moab_loc_discr
                       PROPERTIES COMPILE_FLAGS "${shared_define} -DIS_BUILDING_MB ${MOAB_DEFINES}")

include(AutoconfHeader)
moab_install_headers(${MOAB_LOCALDSRC_Install_Headers})
