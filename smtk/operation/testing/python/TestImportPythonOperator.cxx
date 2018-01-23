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

#include "smtk/operation/ImportPythonOperator.h"
#include "smtk/operation/Manager.h"

int main(int argc, char** const argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: TestImportPythonOperator <operator.py>" << std::endl;
    return 1;
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  test(operationManager->metadata().size() == 0);

  // Register import python operator to the operation manager
  operationManager->registerOperator<smtk::operation::ImportPythonOperator>(
    "smtk::operation::ImportPythonOperator");

  test(operationManager->metadata().size() == 1);

  // Create an import python operator
  smtk::operation::ImportPythonOperator::Ptr importPythonOp =
    operationManager->create<smtk::operation::ImportPythonOperator>();

  // Test that the operator was successfully created
  if (!importPythonOp)
  {
    std::cerr << "No import python operator\n";
    return 1;
  }

  // Set the input python operator file name
  importPythonOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  // Execute the operation
  auto result = importPythonOp->operate();

  // Test the results for success
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "\"import python operator\" operator failed\n";
    return 1;
  }

  test(operationManager->metadata().size() == 2);

  // Access the unique name associated with the operator
  std::string operatorName = result->findString("unique_name")->value();

  // Construct a new operator using its unique name
  smtk::operation::NewOp::Ptr testOp = operationManager->create(operatorName);

  if (!testOp)
  {
    std::cerr << "No " << operatorName << " operator\n";
    return 1;
  }

  // Set a parameter that gets passed from input to output
  testOp->parameters()->findString("my string")->setValue("testing string parameters");

  // Execute the operation
  result = testOp->operate();

  // Test the results for success
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "\"test op\" operator failed\n";
    return 1;
  }

  // Confirm that the operator passed the input string to the output
  test(result->findString("my string")->value() ==
    testOp->parameters()->findString("my string")->value());

  return 0;
}
