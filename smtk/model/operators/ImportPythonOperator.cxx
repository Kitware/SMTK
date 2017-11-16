//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/operators/ImportPythonOperator.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/Paths.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/attribute/FileItem.h"

#include "smtk/io/AttributeReader.h"

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
namespace model
{

smtk::model::OperatorResult ImportPythonOperator::operateInternal()
{
  // Access the python operator's file name
  smtk::attribute::FileItemPtr fileItem = this->findFile("filename");

  // Construct a module name from the file name
  std::string moduleName = smtk::common::Paths::stem(fileItem->value());

  // Load the python source file into our embedded interpreter
  bool success =
    smtk::common::PythonInterpreter::instance().loadPythonSourceFile(fileItem->value(), moduleName);

  if (!success)
  {
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // Query the newly constructed module for SMTK operators
  //
  // TODO: change to check if the operator is a subclass of smtk.operation.Operator
  std::stringstream cmd;
  cmd << "import sys, inspect, smtk, smtk.model, " << moduleName << "\n"
      << "ops = set()\n"
      << "for name, obj in inspect.getmembers(" << moduleName << "):\n"
      << "    if inspect.isclass(obj) and issubclass(obj, smtk.model.Operator):\n"
      << "        op = obj()\n"
      << "        ops.add(op.name())\n"
      << "opstring = ';;'.join(str(op) for op in ops)\n";

  pybind11::dict locals;
  pybind11::exec(cmd.str().c_str(), pybind11::globals(), locals);

  std::string opNames = locals["opstring"].cast<std::string>();

  // As per the above python snippet, the output is a string of all of the
  // operator names defined in the input file, separated by ";;". We parse this
  // string to loop over each python operator.
  regex re(";;");
  sregex_token_iterator first{ opNames.begin(), opNames.end(), re, -1 }, last;

  // If there were no operators in the module, then we failed to import a python
  // operator
  if (first == last)
  {
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // For each operator name in the imported module...
  while (first != last)
  {
    const std::string& opName = *first++;
    // ...check if the operator was statically registered with the current session.
    //
    // TODO: instead of checking this session only, loop over all available
    // sessions and register with each one that has the operator in its static
    // map, if there is an associated session
    if (this->session()->s_operators->find(opName) != this->session()->s_operators->end())
    {
      // This operator was statically registered to the current session. All
      // that remains to make the operator functional is to register its xml
      // description. While the method smtk::model::Session::importOperatorXML()
      // is protected, we have sufficient access to our session's internals to
      // duplicate its functionality.
      smtk::io::AttributeReader rdr;
      bool ok = true;
      const std::string& opXML = (*this->session()->s_operators)[opName].first;
      ok &= !rdr.readContents(
        this->session()->operatorCollection(), opXML.c_str(), opXML.size(), this->log());

      if (!ok)
      {
        std::cerr << "Error. Log follows:\n---\n" << this->log().convertToString() << "\n---\n";
        return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
      }
    }
  }

  return this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
}
}
}

#include "smtk/model/ImportPythonOperator_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::ImportPythonOperator,
  import_python_operator, "import python operator", ImportPythonOperator_xml, smtk::model::Session);
