# Copyright 2007-2009 Brad King, Chuck Stewart
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file rtvl_license_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

set(rtvl_headers
  rtvl_intpow.hxx
  rtvl_terms.hxx
  rtvl_tokens.hxx
  rtvl_vote.hxx                         rtvl_vote.txx
  rtvl_votee.hxx                        rtvl_votee.txx
  rtvl_votee_d.hxx                      rtvl_votee_d.txx
  rtvl_voter.hxx                        rtvl_voter.txx
  rtvl_refine.hxx                       rtvl_refine.txx
  rtvl_tensor.hxx                       rtvl_tensor.txx
  rtvl_tensor_d.hxx                     rtvl_tensor_d.txx
  rtvl_weight.hxx
  rtvl_weight_original.hxx              rtvl_weight_original.txx
  rtvl_weight_smooth.hxx                rtvl_weight_smooth.txx
  )

add_library(rtvl STATIC
  ${rtvl_headers}

  Templates/rtvl_vote+2-.cxx
  Templates/rtvl_votee+2-.cxx
  Templates/rtvl_votee_d+2-.cxx
  Templates/rtvl_voter+2-.cxx
  Templates/rtvl_refine+2-.cxx
  Templates/rtvl_tensor+2-.cxx
  Templates/rtvl_tensor_d+2-.cxx
  Templates/rtvl_weight_original+2-.cxx
  Templates/rtvl_weight_smooth+2-.cxx

  Templates/rtvl_vote+3-.cxx
  Templates/rtvl_votee+3-.cxx
  Templates/rtvl_votee_d+3-.cxx
  Templates/rtvl_voter+3-.cxx
  Templates/rtvl_refine+3-.cxx
  Templates/rtvl_tensor+3-.cxx
  Templates/rtvl_tensor_d+3-.cxx
  Templates/rtvl_weight_original+3-.cxx
  Templates/rtvl_weight_smooth+3-.cxx
  )

#We need to enable position independence on static libraries so we can
#link them to shared libraries.
set_target_properties( rtvl PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

set(ignore_rules_for_rtvl "")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  target_compile_options(rtvl
    PRIVATE
      # Silence warning about std::auto_ptr being deprecated.
      -Wno-deprecated-declarations
      # Silence warning about unused parameters.
      -Wno-unused-parameter)
endif()

target_include_directories(rtvl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
    ${VXL_RPL_INCLUDE_DIR}
    ${VXL_VCL_INCLUDE_DIR}
    ${VXL_CORE_INCLUDE_DIR})

target_link_libraries(rtvl
  LINK_PUBLIC
  ${RGTL_LIBRARY} ${VNL_ALGO_LIBRARY} ${VNL_LIBRARY} ${V3P_NETLIB_LIBRARY})

install(TARGETS rtvl
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(FILES ${rtvl_headers}
  DESTINATION include/rtvl)
