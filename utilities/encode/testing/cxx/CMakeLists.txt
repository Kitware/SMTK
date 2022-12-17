set(encode_tests_without_data
  TestEncode.cxx
)

# Test that with no INCLUDE_DIRS, no substitution is performed.
add_custom_command(
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/input1.xml
  DEPENDS smtk_encode_file
  COMMAND smtk_encode_file
  ARGS
    TYPE xml EXT h NAME input1_xml
    HEADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_xml.h
    ${CMAKE_CURRENT_SOURCE_DIR}/input1.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_xml.h
)

# Test that C++ functions produce data identical to the literal,
# even when chopped into small pieces and re-assembled.
add_custom_command(
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/input1.xml
  DEPENDS smtk_encode_file
  COMMAND smtk_encode_file
  ARGS
    TYPE cpp EXT h NAME input1_cpp PIECE_SIZE 64
    HEADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_cpp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/input1.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_cpp.h
)

# Test that include directives are handled properly.
add_custom_command(
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/input1.sbt
  DEPENDS smtk_encode_file
  COMMAND smtk_encode_file
  ARGS
    INCLUDE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
      TYPE xml EXT h NAME input1_sbt
      HEADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_sbt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/input1.sbt
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/input1_sbt.h
)

# Depend on generated files.
set(encode_extra_source
  ${CMAKE_CURRENT_BINARY_DIR}/input1_xml.h
  ${CMAKE_CURRENT_BINARY_DIR}/input1_cpp.h
  ${CMAKE_CURRENT_BINARY_DIR}/input1_sbt.h
)

smtk_unit_tests(
  LABEL "EncodeFiles"
  SOURCES ${encode_tests_without_data}
  SOURCES_REQUIRE_DATA ${encode_tests_which_require_data}
  EXTRA_SOURCES ${encode_extra_source}
  LIBRARIES Boost::filesystem
)
