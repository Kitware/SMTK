# Project SaveAs Function

A method smtk::project::Manager::SaveAs() has been added to the project
subsystem for saving the current project to a new filesystem directory.
On success, the project manager closes the current project and makes the
copy the current project. The application is responsible for saving the
original project, if needed, *before* calling the SaveAs method.
