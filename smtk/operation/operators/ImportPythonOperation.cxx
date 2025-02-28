//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/operators/ImportPythonOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/Paths.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/pybind11/PyOperation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/Regex.h"

#include <set>
#include <string>

namespace smtk
{
namespace operation
{

bool ImportPythonOperation::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To import a python operation, we must have an operation manager into which
  // the new operation is imported.
  if (m_manager.expired())
  {
    return false;
  }

  return true;
}

std::vector<std::string> ImportPythonOperation::importOperationsFromModule(
  const std::string& moduleName,
  smtk::operation::Manager& manager)
{
  // Query the module for SMTK operations
  std::stringstream cmd;
  cmd << "import sys, inspect, smtk, smtk.operation, " << moduleName << "\n"
      << "ops = set()\n"
      << "for name, obj in inspect.getmembers(" << moduleName << "):\n"
      << "    if inspect.isclass(obj) and issubclass(obj, smtk.operation.Operation):\n"
      << "        ops.add(obj.__name__)\n"
      << "opstring = ';;'.join(str(op) for op in ops)\n";

  pybind11::dict locals;
  pybind11::exec(cmd.str().c_str(), pybind11::globals(), locals);

  std::string opNames = locals["opstring"].cast<std::string>();

  if (opNames.empty())
  {
    // There were no operations in the module
    return std::vector<std::string>();
  }

  // As per the above python snippet, the output is a string of all of the
  // operation names defined in the input file, separated by ";;". We parse this
  // string to loop over each python operation.
  smtk::regex re(";;");
  smtk::sregex_token_iterator first{ opNames.begin(), opNames.end(), re, -1 }, last;

  std::vector<std::string> typeNames;

  // For each operation name in the imported module...
  while (first != last)
  {
    const std::string& opName = *first++;

    bool registered = importOperation(manager, moduleName, opName);

    if (registered)
    {
      typeNames.emplace_back(moduleName + "." + opName);
    }
  }

  return typeNames;
}

bool ImportPythonOperation::importOperation(
  smtk::operation::Manager& manager,
  const std::string& moduleName,
  const std::string& opName)
{
  std::string typeName = moduleName + "." + opName;
  smtk::operation::Operation::Index index = std::hash<std::string>{}(typeName);
  auto create = std::bind(smtk::operation::PyOperation::create, moduleName, opName, index);

  auto specification = create()->createSpecification();

  return manager.registerOperation(Metadata(typeName, index, specification, create));
}

ImportPythonOperation::Result ImportPythonOperation::operateInternal()
{
  // Access the python operation's file name
  smtk::attribute::FileItemPtr fileItem = this->parameters()->findFile("filename");

  // Construct a module name from the file name
  std::string moduleName = smtk::common::Paths::stem(fileItem->value());

  // Load the python source file into our embedded interpreter
  bool success =
    common::PythonInterpreter::instance().loadPythonSourceFile(fileItem->value(), moduleName);

  if (!success)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::vector<std::string> typeNames =
    ImportPythonOperation::importOperationsFromModule(moduleName, *(m_manager.lock()));

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::StringItemPtr typeNameItem = result->findString("unique_name");

  for (auto& typeName : typeNames)
  {
    typeNameItem->appendValue(typeName);
  }

  return typeNameItem->numberOfValues() > 0
    ? result
    : this->createResult(smtk::operation::Operation::Outcome::FAILED);
}

void ImportPythonOperation::generateSummary(Result& result)
{
  std::ostringstream s;
  if (result)
  {
    auto opNameItem = result->findString("unique_name");
    if (opNameItem)
    {
      bool once = true;
      for (const auto& opName : *opNameItem)
      {
        if (once)
        {
          once = false;
        }
        else
        {
          s << "\n";
        }
        s << "Imported \"" << opName << "\".";
      }
    }
  }
  if (outcome(result) == Outcome::SUCCEEDED)
  {
    smtkInfoMacro(this->log(), s.str());
  }
  else
  {
    smtkErrorMacro(this->log(), s.str());
  }
}

ImportPythonOperation::Specification ImportPythonOperation::createSpecification()
{
  Specification spec = this->createBaseSpecification();

  auto opDef = spec->createDefinition("import python op", "operation");
  opDef->setBriefDescription("Import a python operation.");

  const char detailedDescription[] =
    "<p>A class for adding python operations to the current session."
    "<p>Given a python file that describes an operation, this operation loads the"
    "python operation into its operation manager. The new operation is ready for"
    "use upon the successful completion of this operation.";

  opDef->setDetailedDescription(std::string(detailedDescription));

  auto fileDef = smtk::attribute::FileItemDefinition::New("filename");
  fileDef->setNumberOfRequiredValues(1);
  fileDef->setShouldExist(true);
  fileDef->setFileFilters("Python (*.py)");

  opDef->addItemDefinition(fileDef);

  auto resultDef = spec->createDefinition("result(import python operation)", "result");

  auto stringDef = smtk::attribute::StringItemDefinition::New("unique_name");
  stringDef->setNumberOfRequiredValues(0);
  stringDef->setIsExtensible(true);
  resultDef->addItemDefinition(stringDef);

  return spec;
}
} // namespace operation
} // namespace smtk
