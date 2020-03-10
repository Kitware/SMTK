### smtk_operation_xml() replaced with smtk_export_file()

When operations add a .sbt xml file to specify the interface of the operation,
it was wrapped into a header file by the cmake function
`smtk_operation_xml()`.  Update operations to use `smtk_encode_file()`,
because it will generate the file at build time instead of cmake configure
time, and the build tool (like `ninja`) will pick up on changes to the .sbt
file and rebuild.
