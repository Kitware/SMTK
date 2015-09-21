set(moabverdict_srcs
  V_EdgeMetric.cpp
  V_GaussIntegration.cpp
  V_HexMetric.cpp
  V_KnifeMetric.cpp
  V_PyramidMetric.cpp
  V_QuadMetric.cpp
  V_TetMetric.cpp
  V_TriMetric.cpp
  VerdictVector.cpp
  V_WedgeMetric.cpp
  verdict_defines.hpp
  VerdictVector.hpp
  v_vector.h
  V_GaussIntegration.hpp
  VerdictWrapper.cpp)

set(moabverdict_headers
  moab/verdict.h
  moab/VerdictWrapper.hpp)

add_definitions(-DIS_BUILDING_MB)
add_definitions("-DSRCDIR=${CMAKE_CURRENT_SOURCE_DIR}")

add_library(moabverdict OBJECT
  ${moabverdict_srcs}
  ${moabverdict_headers})

# Set position-independent flag so that the object files are embedded properly.
set_target_properties(moabverdict PROPERTIES POSITION_INDEPENDENT_CODE True)

if (BUILD_SHARED_LIBS)
  set( shared_define "-DIS_BUILDING_MB_SHARED" )
endif ()
set_target_properties(moabverdict PROPERTIES COMPILE_FLAGS "${shared_define}")
include(AutoconfHeader)
moab_install_headers(${moabverdict_headers})
