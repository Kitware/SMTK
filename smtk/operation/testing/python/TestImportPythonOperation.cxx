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

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/operation/ImportPythonOperation.h"
#include "smtk/operation/Manager.h"

int main(int argc, char** const argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: TestImportPythonOperation <operation.py>" << std::endl;
    return 1;
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  test(operationManager->metadata().size() == 0);

  // Register import python operation to the operation manager
  operationManager->registerOperation<smtk::operation::ImportPythonOperation>(
    "smtk::operation::ImportPythonOperation");

  test(operationManager->metadata().size() == 1);

  // Create an import python operation
  smtk::operation::ImportPythonOperation::Ptr importPythonOp =
    operationManager->create<smtk::operation::ImportPythonOperation>();

  // Test that the operation was successfully created
  if (!importPythonOp)
  {
    std::cerr << "No import python operation\n";
    return 1;
  }

  // Set the input python operation file name
  importPythonOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  auto result = importPythonOp->operate();

  // Test the results for success
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"import python operation\" operation failed\n";
    return 1;
  }

  test(operationManager->metadata().size() == 2);

  // Access the unique name associated with the operation
  std::string operationName = result->findString("unique_name")->value();

  // Construct a new operation using its unique name
  smtk::operation::Operation::Ptr testOp = operationManager->create(operationName);

  if (!testOp)
  {
    std::cerr << "No " << operationName << " operation\n";
    return 1;
  }

  // Set a parameter that gets passed from input to output
  testOp->parameters()->findString("my string")->setValue("testing string parameters");

  // Execute the operation
  result = testOp->operate();

  // Test the results for success
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"test op\" operation failed\n";
    return 1;
  }

  // Confirm that the operation passed the input string to the output
  test(result->findString("my string")->value() ==
    testOp->parameters()->findString("my string")->value());

  return 0;
}
