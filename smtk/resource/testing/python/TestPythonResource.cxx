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

#include "smtk/resource/Manager.h"
#include "smtk/resource/RegisterPythonResource.h"

#include <string>

int main(int argc, char** const argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: TestPythonResource <resource.py>" << std::endl;
    return 1;
  }

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  test(resourceManager->metadata().empty());

  // Construct a module name from the file name
  std::string moduleName = smtk::common::Paths::stem(std::string(argv[1]));

  // Load the python source file into our embedded interpreter
  bool success = smtk::common::PythonInterpreter::instance().loadPythonSourceFile(
    std::string(argv[1]), moduleName);

  test(success, "Failed to load python file.");

  // Register the resource defined in the module
  success = smtk::resource::registerPythonResource(resourceManager, moduleName);

  test(success, "Failed to register python resource.");

  test(resourceManager->metadata().size() == 1, "No metdata entry for new resource");

  // Access its type name from its registered metadata
  std::string typeName;
  for (auto& metadatum : resourceManager->metadata())
    typeName = metadatum.typeName();

  // Construct an instance of the resource
  smtk::resource::Resource::Ptr resource = resourceManager->create(typeName);

  test(resource != nullptr, "Failed to create Python resource");

  // Compose the resource's name from the class name and UUID. This is how the
  // name() method is defined in the Python description of the Resource.
  std::string name =
    typeName.substr(typeName.find_first_of('.') + 1) + " (" + resource->id().toString() + ")";

  test(resource->name() == name, "Failed to call Python derived method from C++.");

  return 0;
}
