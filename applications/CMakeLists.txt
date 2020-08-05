if (SMTK_ENABLE_POLYGON_SESSION)
  add_subdirectory(TemplateEditor)
endif()

# Because CPack assumes there is only one project to package, ModelViewer is not
# included as a subdirectory. Instead, it is included as a contract test that
# demonstrates how to build and package the application.
