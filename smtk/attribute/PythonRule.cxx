//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/PythonRule.h"

#include "smtk/attribute/Definition.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/PythonInterpreter.h"

#include "smtk/io/Logger.h"

SMTK_THIRDPARTY_PRE_INCLUDE
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

#include <pybind11/embed.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <exception>

namespace smtk
{
namespace attribute
{

bool PythonRule::operator()(
  const Attribute::ConstPtr& attribute,
  const smtk::resource::PersistentObject::ConstPtr& object) const
{
  // Initialize SMTK's embedded interpreter. This will locate SMTK's Python
  // bindings and ensure that they can be imported.
  smtk::common::PythonInterpreter::instance().initialize();

  // Import all associated source files.
  for (std::string sourceFile : m_sourceFiles)
  {
    // If the source file is described as a relative path, attempt to find it
    // relative to the location of the attribute resource.
    if (!boost::filesystem::path(sourceFile).is_absolute())
    {
      sourceFile =
        (boost::filesystem::path(attribute->resource()->location()).parent_path() / sourceFile)
          .string();
    }

    if (
      !boost::filesystem::is_regular_file(boost::filesystem::path(sourceFile)) &&
      !boost::filesystem::is_symlink(boost::filesystem::path(sourceFile)))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Could not locate Python source file \"" << sourceFile << "\".");
    }
    else if (!smtk::common::PythonInterpreter::instance().loadPythonSourceFile(sourceFile))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Could not load Python source file \"" << sourceFile << "\".");
    }
  }

  // The inputs to our Python function will be the input attribute and object.
  auto locals = pybind11::dict();
  pybind11::module module = pybind11::module::import("smtk.attribute");
  locals["attribute"] = attribute;
  locals["object"] = object;

  // We assume that the final Python function described in m_functionString
  // is the function we want to call, so we determine its name.
  std::string functionName;
  auto functionNameStart = m_functionString.rfind("def ");
  if (functionNameStart != std::string::npos)
  {
    functionNameStart += 4;
    functionName = m_functionString.substr(
      functionNameStart,
      m_functionString.find_first_of('(', functionNameStart) - functionNameStart);
  }

  if (functionName.empty())
  {
    // If we cannot find the call function, we log the Python function and treat
    // this rule as an all-pass filter.
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not determine Python function to call from the following:\n\n"
        << m_functionString);
    return true;
  }

  // We append a single line to execute the Python function and assign its return
  // value to a variable we can extract.
  std::stringstream s;
  s << m_functionString << "\n";
  s << "returnValue = " << functionName << "(attribute, object)";

  try
  {
    pybind11::exec(s.str().c_str(), pybind11::globals(), locals);
  }
  catch (std::exception& e)
  {
    // If something went wrong with the Python execution, we log the Python
    // function and the error and treat this rule as an all-pass filter.
    s << "\n\n" << e.what();
    smtk::io::Logger::instance().addRecord(
      smtk::io::Logger::Error,
      "PythonRule for definition \"" + attribute->definition()->type() +
        "\" encountered an error:\n" + s.str());
    return true;
  }

  return locals["returnValue"].cast<bool>();
}

const PythonRule& PythonRule::operator>>(nlohmann::json& json) const
{
  // There are a handful of reserved characters in json. Since our json files
  // do not need to be human-readable, one workaround is to encode our Python
  // string into something more digestible. Luckily, Python has routines to do
  // just this.

  json["SourceFiles"] = m_sourceFiles;

  auto locals = pybind11::dict();
  locals["functionStr"] = m_functionString;

  const char* encode = R"python(
import base64
returnValue = base64.b64encode(functionStr.encode('ascii')))python";

  try
  {
    pybind11::exec(encode, pybind11::globals(), locals);
    json["Function"] = locals["returnValue"].cast<std::string>();
  }
  catch (std::exception&)
  {
  }

  return *this;
}

PythonRule& PythonRule::operator<<(const nlohmann::json& json)
{
  // See the explanation in operator>>() above.

  m_sourceFiles = json["SourceFiles"].get<std::vector<std::string>>();

  auto locals = pybind11::dict();
  locals["b64encodedStr"] = json["Function"].get<std::string>();

  const char* decode = R"python(
import base64
returnValue = base64.b64decode(b64encodedStr).decode('ascii'))python";

  try
  {
    pybind11::exec(decode, pybind11::globals(), locals);
    m_functionString = locals["returnValue"].cast<std::string>();
  }
  catch (std::exception&)
  {
  }

  return *this;
}

const PythonRule& PythonRule::operator>>(pugi::xml_node& node) const
{
  if (!m_sourceFiles.empty())
  {
    pugi::xml_node subnode = node.append_child("SourceFiles");
    for (const auto& sourceFile : m_sourceFiles)
    {
      subnode.append_child("SourceFile").text().set(sourceFile.c_str());
    }
  }

  // There are a handful of reserved characters in XML. We avoid complications
  // between these reserved characters and Python syntax by treating our Python
  // input as CDATA, which the parser ignores.
  node.append_child(pugi::node_cdata).set_value(m_functionString.c_str());
  return *this;
}

PythonRule& PythonRule::operator<<(const pugi::xml_node& node)
{
  m_sourceFiles.clear();

  pugi::xml_node subnode = node.child("SourceFiles");
  if (subnode)
  {
    for (pugi::xml_node child = subnode.child("SourceFile"); child;
         child = child.next_sibling("SourceFile"))
    {
      m_sourceFiles.emplace_back(child.text().get());
    }
  }

  // See the explanation in operator>>() above.
  m_functionString = node.text().data().value();
  return *this;
}
} // namespace attribute
} // namespace smtk
