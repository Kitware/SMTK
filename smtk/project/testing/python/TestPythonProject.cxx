//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/Paths.h"
#include "smtk/common/PythonInterpreter.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/operation/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/RegisterPythonProject.h"

#include "smtk/resource/Manager.h"

#include <string>

int main(int argc, char** const argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: TestPythonProject <project.py>" << std::endl;
    return 1;
  }

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Create a project manager
  smtk::project::Manager::Ptr projectManager =
    smtk::project::Manager::create(resourceManager, operationManager);

  // Construct a module name from the file name
  std::string moduleName = smtk::common::Paths::stem(std::string(argv[1]));

  // Load the python source file into our embedded interpreter
  bool success = smtk::common::PythonInterpreter::instance().loadPythonSourceFile(
    std::string(argv[1]), moduleName);

  test(success, "Failed to load python file.");

  // Register the project defined in the module
  success = smtk::project::registerPythonProject(projectManager, moduleName);

  test(success, "Failed to register python project.");

  test(projectManager->metadata().size() == 1, "No metdata entry for new project");

  // Access its type name from its registered metadata
  std::string typeName;
  for (const auto& metadatum : projectManager->metadata())
    typeName = metadatum.typeName();

  // Construct an instance of the project
  smtk::project::Project::Ptr project = projectManager->create(typeName);

  test(project != nullptr, "Failed to create Python project");

  // Compose the project's name from the class name and UUID. This is how the
  // name() method is defined in the Python description of the Project.
  std::string name =
    typeName.substr(typeName.find_first_of('.') + 1) + " (" + project->id().toString() + ")";

  std::cout << "project name: " << project->name() << std::endl;
  std::cout << "expected: " << name << std::endl;
  test(project->name() == name, "Failed to call Python derived method from C++.");

  return 0;
}
