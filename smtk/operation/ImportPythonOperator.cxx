//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/operation/ImportPythonOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/Paths.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/pybind11/PyOperator.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/regex.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

#include <set>
#include <string>

namespace smtk
{
namespace operation
{

bool ImportPythonOperator::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  // To import a python operator, we must have an operation manager into which
  // the new operator is imported.
  if (m_manager == nullptr)
  {
    return false;
  }

  return true;
}

std::vector<std::string> ImportPythonOperator::importOperatorsFromModule(
  const std::string& moduleName, smtk::operation::Manager& manager)
{
  // Query the module for SMTK operators
  std::stringstream cmd;
  cmd << "import sys, inspect, smtk, smtk.operation, " << moduleName << "\n"
      << "ops = set()\n"
      << "for name, obj in inspect.getmembers(" << moduleName << "):\n"
      << "    if inspect.isclass(obj) and issubclass(obj, smtk.operation.NewOp):\n"
      << "        ops.add(obj.__name__)\n"
      << "opstring = ';;'.join(str(op) for op in ops)\n";

  pybind11::dict locals;
  pybind11::exec(cmd.str().c_str(), pybind11::globals(), locals);

  std::string opNames = locals["opstring"].cast<std::string>();

  if (opNames.empty())
  {
    // There were no operators in the module
    return std::vector<std::string>();
  }

  // As per the above python snippet, the output is a string of all of the
  // operator names defined in the input file, separated by ";;". We parse this
  // string to loop over each python operator.
  regex re(";;");
  sregex_token_iterator first{ opNames.begin(), opNames.end(), re, -1 }, last;

  std::vector<std::string> uniqueNames;

  // For each operator name in the imported module...
  while (first != last)
  {
    const std::string& opName = *first++;

    bool registered = importOperator(manager, moduleName, opName);

    if (registered)
    {
      uniqueNames.push_back(std::string(moduleName + "." + opName));
    }
  }

  return uniqueNames;
}

bool ImportPythonOperator::importOperator(
  smtk::operation::Manager& manager, const std::string& moduleName, const std::string& opName)
{
  std::string uniqueName = moduleName + "." + opName;
  smtk::operation::NewOp::Index index = std::hash<std::string>{}(uniqueName);
  auto create = std::bind(smtk::operation::PyOperator::create, moduleName, opName, index);

  return manager.registerOperator(
    Metadata(uniqueName, index, create()->createSpecification(), create));
}

ImportPythonOperator::Result ImportPythonOperator::operateInternal()
{
  // Access the python operator's file name
  smtk::attribute::FileItemPtr fileItem = this->parameters()->findFile("filename");

  // Construct a module name from the file name
  std::string moduleName = smtk::common::Paths::stem(fileItem->value());

  // Load the python source file into our embedded interpreter
  bool success =
    smtk::common::PythonInterpreter::instance().loadPythonSourceFile(fileItem->value(), moduleName);

  if (!success)
  {
    return this->createResult(smtk::operation::NewOp::Outcome::FAILED);
  }

  std::vector<std::string> uniqueNames =
    this->importOperatorsFromModule(moduleName, *(this->m_manager));

  Result result = this->createResult(smtk::operation::NewOp::Outcome::SUCCEEDED);

  smtk::attribute::StringItemPtr uniqueNameItem = result->findString("unique_name");

  for (auto& uniqueName : uniqueNames)
  {
    uniqueNameItem->appendValue(uniqueName);
  }

  return uniqueNameItem->numberOfValues() > 0 ? result : this->createResult(
                                                           smtk::operation::NewOp::Outcome::FAILED);
}

ImportPythonOperator::Specification ImportPythonOperator::createSpecification()
{
  Specification spec = this->createBaseSpecification();

  auto opDef = spec->createDefinition("test op", "operator");
  opDef->setBriefDescription("Import a python operator.");

  const char detailedDescription[] =
    "<p>A class for adding python operators to the current session."
    "<p>Given a python file that describes an operator, this operator loads the"
    "python operator into its operation manager. The new operator is ready for"
    "use upon the successful completion of this operation.";

  opDef->setDetailedDescription(std::string(detailedDescription));

  auto fileDef = smtk::attribute::FileItemDefinition::New("filename");
  fileDef->setNumberOfRequiredValues(1);
  fileDef->setShouldExist(true);
  fileDef->setFileFilters("Python (*.py)");

  opDef->addItemDefinition(fileDef);

  auto resultDef = spec->createDefinition("result(import python operator)", "result");

  auto stringDef = smtk::attribute::StringItemDefinition::New("unique_name");
  stringDef->setNumberOfRequiredValues(0);
  stringDef->setIsExtensible(true);
  resultDef->addItemDefinition(stringDef);

  return spec;
}
}
}
