add_executable(token token.cxx)
target_link_libraries(token PUBLIC smtkCore)

add_executable(markup_dump markup_dump.cxx)
target_link_libraries(markup_dump
  PUBLIC
    smtkMarkup
    smtkCore
)
install(TARGETS markup_dump
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION Applications      # Required for MACOSX_BUNDLE
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
