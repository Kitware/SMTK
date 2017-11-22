//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/embed.h>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

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
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

#include <boost/dll.hpp>

#if WIN32
#include <Python.h>
extern __declspec(dllimport) int Py_NoSiteFlag;
#endif

#include <patchlevel.h>

SMTK_THIRDPARTY_POST_INCLUDE

// We include the header after the third party includes because Python.h warns
// if it is not included before the stl headers our header file includes.
#include "smtk/common/PythonInterpreter.h"

#include "smtk/common/Paths.h"

#include <cstdlib>
#include <sstream>

namespace smtk
{
namespace common
{
PythonInterpreter PythonInterpreter::m_instance;

PythonInterpreter& PythonInterpreter::instance()
{
  return PythonInterpreter::m_instance;
}

PythonInterpreter::PythonInterpreter()
  : m_embedded(false)
{
  this->initialize();
}

PythonInterpreter::~PythonInterpreter()
{
  this->finalize();
}

bool PythonInterpreter::isInitialized() const
{
  return Py_IsInitialized() != 0;
}

void PythonInterpreter::initialize()
{
  // If the interpreter is already initialized, then there's nothing to do.
  if (this->isInitialized())
  {
    return;
  }

  // We are initializing the embedded python.
  m_embedded = true;

  // Locate the directory containing the python library in use, and set
  // PYTHONHOME to this path.
  static std::string pythonLibraryLocation = Paths::pathToLibraryContainingFunction(Py_Initialize);
#if (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3) || PY_MAJOR_VERSION > 3
  // Python 3.3 switched to wchar_t.
  static std::vector<wchar_t> loc;
  loc.resize(pythonLibraryLocation.size() + 1);
  mbstowcs(&loc[0], pythonLibraryLocation.c_str(), static_cast<size_t>(loc.size()));
  Py_SetProgramName(&loc[0]);
#else
  Py_SetProgramName(const_cast<char*>(pythonLibraryLocation.c_str()));
#endif

  // Initialize the embedded interpreter.
  Py_NoSiteFlag = 1;
  pybind11::initialize_interpreter();

  // If the executing process's environment has set PYTHONPATH to find smtk,
  // then there's no need for us to look for it.
  if (canFindModule("smtk"))
  {
    return;
  }

  // Otherwise, locate the directory containing the library that describes this
  // class.
  boost::filesystem::path smtkLibDir =
    boost::dll::symbol_location(PythonInterpreter::instance).parent_path();

  // We first look for SMTK as run from the build tree.
  bool smtkFound = this->addPathToBuildTree(smtkLibDir.parent_path().string(), "smtk");

  // If we don't find it, then we look for SMTK as an installed module.
  if (!smtkFound)
  {
    smtkFound = this->addPathToInstalledModule(smtkLibDir.string(), "smtk");
  }

  // If we don't find it, then we look for SMTK as a packaged module.
  if (!smtkFound)
  {
    smtkFound = this->addPathToPackagedModule(smtkLibDir.string(), "smtk");
  }

  // If we still don't find it, we don't do anyting special. Consuming projects
  // (like CMB) may have packaged SMTK with logic that does not follow SMTK's
  // install or build pattern, and that's ok. In this case, it is up to the
  // consuming project to properly set the embedded python's PYTHONPATH to find
  // SMTK, and can use the public methods pathToLibraryContainingFunction(),
  // addToPythonPath() and canFindModule() to do so.
}

void PythonInterpreter::finalize()
{
  if (this->isInitialized())
  {
    pybind11::finalize_interpreter();
    m_embedded = false;
  }
}

std::vector<std::string> PythonInterpreter::pythonPath()
{
  if (!this->isInitialized())
  {
    this->initialize();
  }

  // Access the embedded python's PYTHONPATH from the python instance itself,
  // and parse it into a vector of strings.
  pybind11::dict locals;
  std::stringstream testCmd;
  testCmd << "import sys\n"
          << "path = ','.join(sys.path)";
  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  std::string path_list = locals["path"].cast<std::string>();

  std::vector<std::string> paths;

  regex re(",");
  sregex_token_iterator it(path_list.begin(), path_list.end(), re, -1), last;
  for (; it != last; ++it)
  {
    paths.push_back(it->str());
  }

  return paths;
}

bool PythonInterpreter::addToPythonPath(const std::string& path_list, std::string separator)
{
  if (!this->isInitialized())
  {
    this->initialize();
  }

  // Iterate over <path_list> with breaks at <separator> and add each path to
  // the embedded python's PYTHONPATH.
  pybind11::module sys = pybind11::module::import("sys");
  regex re(separator);
  sregex_token_iterator it(path_list.begin(), path_list.end(), re, -1), last;
  for (; it != last; ++it)
  {
    sys.attr("path").attr("insert")(0, it->str().c_str());
  }

  return true;
}

bool PythonInterpreter::canFindModule(const std::string& module) const
{
  // If the python interpreter is not initialized, then we cannot find any
  // modules.
  if (!this->isInitialized())
  {
    return false;
  }

  // Within the embedded python environment, attempt to import <module>. We
  // allow for the possibility of the module not being found, but we
  // deliberately do not catch additional exceptions in case <module> cannot be
  // loaded for other reasons.

  bool found = true;
  pybind11::dict locals;
  std::stringstream testCmd;
  testCmd << "found = True\n"
          << "try:\n"
          << "    import " << module << "\n"
          << "except ImportError:\n"
          << "    found = False";
  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  found = locals["found"].cast<bool>();

  return found;
}

bool PythonInterpreter::addPathToPackagedModule(
  const std::string& libPackageDir, const std::string& module)
{
#ifdef SMTK_MSVC
  // If <module> is run out of a package, we expect that the directory that
  // contains its libraries to contain "Lib/site-packages/<module>", so we
  // attempt to add this directory to the PYTHONPATH.

  boost::filesystem::path bundledPyInit =
    boost::filesystem::path(libPackageDir) / "Lib" / "site-packages" / module / "__init__.py";

  if (boost::filesystem::is_regular_file(bundledPyInit))
  {
    this->addToPythonPath(bundledPyInit.parent_path().parent_path().string());
    return true;
  }
#else
  // If <module> is run out of a package, we expect that the directory that
  // contains its libraries is at the same level as "Python/<module>", so we
  // attempt to add this directory to the PYTHONPATH.

  boost::filesystem::path bundledPyInit =
    boost::filesystem::path(libPackageDir).parent_path() / "Python" / module / "__init__.py";

  if (boost::filesystem::is_regular_file(bundledPyInit))
  {
    this->addToPythonPath(bundledPyInit.parent_path().parent_path().string());
    return true;
  }
#endif
  return false;
}

bool PythonInterpreter::addPathToInstalledModule(
  const std::string& libInstallDir, const std::string& module)
{
  // If <module> is run out of the install tree, we expect that the directory
  // that contains its libraries also contains a directory called
  // "python<PY_MAJOR_VERSION>.<PY_MINOR_VERSION>"/site-packages/<module>", so
  // we attempt to add this directory to the PYTHONPATH.

  std::stringstream installedPyInitStream;
  installedPyInitStream << libInstallDir << "/"
                        << "python" << PY_MAJOR_VERSION << "." << PY_MINOR_VERSION
                        << "/site-packages/" << module << "/__init__.py";
  boost::filesystem::path installedPyInit(installedPyInitStream.str());

  if (boost::filesystem::is_regular_file(installedPyInit))
  {
    this->addToPythonPath(installedPyInit.parent_path().parent_path().string());
    return true;
  }
  return false;
}

bool PythonInterpreter::addPathToBuildTree(
  const std::string& buildTreePath, const std::string& module)
{
  // If <module> is run out of the build tree, we expect the directory that
  // contains its libraries to reside one level below the build directory,
  // so we attempt to add the build directory to the PYTHONPATH.

  boost::filesystem::path buildTreePyInit =
    boost::filesystem::path(buildTreePath) / module / "__init__.py";

  if (boost::filesystem::is_regular_file(buildTreePyInit))
  {
    this->addToPythonPath(buildTreePath);
    return true;
  }
  return false;
}

void PythonInterpreter::dontWriteByteCode(bool choice)
{
  // Access the embedded python's sys.dont_write_byte_code from the python
  // instance itself.
  pybind11::dict locals;
  std::stringstream testCmd;
  testCmd << "import sys\n"
          << "sys.dont_write_bytecode = " << (choice ? "True" : "False");
  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);
}

bool PythonInterpreter::dontWriteByteCode()
{
  // Access the embedded python's sys.dont_write_byte_code from the python
  // instance itself.
  pybind11::dict locals;
  std::stringstream testCmd;
  testCmd << "import sys\n"
          << "choice = sys.dont_write_bytecode";
  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  bool choice = locals["choice"].cast<bool>();

  return choice;
}

bool PythonInterpreter::loadPythonSourceFile(const std::string& fileName)
{
  return this->loadPythonSourceFile(fileName, smtk::common::Paths::stem(fileName));
}

bool PythonInterpreter::loadPythonSourceFile(
  const std::string& fName, const std::string& moduleName)
{
  // Convert to boost's generic format to avoid complications with Windows slashes
  std::string fileName = boost::filesystem::path(fName).generic_string();

  // For user-submitted source files, disable the generation of byte code
  bool dontWriteByteCode = this->dontWriteByteCode();
  this->dontWriteByteCode(true);

  bool loaded = true;
  pybind11::dict locals;
  std::stringstream testCmd;

  testCmd << "loaded = True\n"
          << "try:\n"
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 5
          << "    import importlib.util\n"
          << "    spec = importlib.util.spec_from_file_location('" << moduleName << "', '"
          << fileName << "')\n"
          << "    tmp = importlib.util.module_from_spec(spec)\n"
          << "    spec.loader.exec_module(tmp)\n"
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3 && PY_MINOR_VERSION <= 4
          << "    from importlib.machinery import SourceFileLoader\n"
          << "    tmp = SourceFileLoader('" << moduleName << "', '" << fileName
          << "').load_module()\n"
#else /* PY_MAJOR_VERSION == 2 */
          << "    import imp\n"
          << "    tmp = imp.load_source('" << moduleName << "', '" << fileName << "')\n"
#endif
          << "except:\n"
          << "    loaded = False";

  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  loaded = locals["loaded"].cast<bool>();

  this->dontWriteByteCode(dontWriteByteCode);

  return loaded;
}
}
}
