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

#include <boost/dll.hpp>

#ifdef _WIN32
#include <Python.h>
extern __declspec(dllimport) int Py_NoSiteFlag;
#endif

#include <patchlevel.h>

SMTK_THIRDPARTY_POST_INCLUDE

// We include the header after the third party includes because Python.h warns
// if it is not included before the stl headers our header file includes.
#include "smtk/common/PythonInterpreter.h"

#include "smtk/common/Environment.h"
#include "smtk/common/Paths.h"

#include "smtk/Regex.h"

#include <cstdlib>
#include <sstream>

namespace
{

//PYTHON_MODULEDIR is defined by cmake
std::string python_moduledir = PYTHON_MODULEDIR;
} // namespace

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

#ifdef __APPLE__
  // On macOS, see if we are running inside a packaged bundle.
  // If so, set the python path appropriately
  smtk::common::Paths paths;
  auto bundleDir = paths.bundleDirectory();
  std::ostringstream bundlePythonPath;
  bundlePythonPath << bundleDir << "/Contents/Libraries/lib/python" << PY_MAJOR_VERSION << "."
                   << PY_MINOR_VERSION;
  if (paths.directoryExists(bundlePythonPath.str()))
  {
    std::vector<wchar_t> pythonPath;
    pythonPath.resize(bundlePythonPath.str().size() + 1);
    mbstowcs(
      &pythonPath[0], bundlePythonPath.str().c_str(), static_cast<size_t>(pythonPath.size()));
    Py_SetPath(&pythonPath[0]);
  }
#endif

  // Locate the directory containing the python library in use, and set
  // PYTHONHOME to this path.
  static std::string pythonLibraryLocation = Paths::pathToLibraryContainingFunction(Py_Initialize);
#if (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3) || PY_MAJOR_VERSION > 3
  // Python 3.3 switched to wchar_t.
  static std::vector<wchar_t> loc;
  loc.resize(pythonLibraryLocation.size() + 1);
  mbstowcs(loc.data(), pythonLibraryLocation.c_str(), static_cast<size_t>(loc.size()));
  Py_SetProgramName(loc.data());
#else
  Py_SetProgramName(const_cast<char*>(pythonLibraryLocation.c_str()));
#endif

  // Initialize the embedded interpreter.
  Py_NoSiteFlag = 1;
  pybind11::initialize_interpreter();

  // Locate the directory containing the library that describes this
  // class.
  boost::filesystem::path smtkLibDir =
    boost::dll::symbol_location(PythonInterpreter::instance).parent_path();

  // Use it to ensure that the smtk module is in our embedded python instance's
  // module path. If we don't find it, we don't do anyting special. Consuming
  // projects (like CMB) may have packaged SMTK with logic that does not follow
  // SMTK's install or build pattern, and that's ok. In this case, it is up to
  // the consuming project to properly set the embedded python's PYTHONPATH to
  // find SMTK, and can use the public methods
  // pathToLibraryContainingFunction(), addToPythonPath() and canFindModule() to
  // do so.
  this->addPathToPluginModule("smtk", smtkLibDir.string());
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

  smtk::regex re(",");
  smtk::sregex_token_iterator it(path_list.begin(), path_list.end(), re, -1), last;
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
  smtk::regex re(separator);
  smtk::sregex_token_iterator it(path_list.begin(), path_list.end(), re, -1), last;
  for (; it != last; ++it)
  {
    sys.attr("path").attr("insert")(0, it->str().c_str());
  }

  return true;
}

bool PythonInterpreter::addPathToPluginModule(const std::string& module, const std::string& libdir)
{
  // If the executing process's environment has set PYTHONPATH to find the
  // module, then there's no need for us to look for it.
  if (canFindModule(module))
  {
    return true;
  }

  std::string dir = boost::filesystem::path(libdir).parent_path().string();

  // Otherwise, locate the directory containing a library in the plugin.
  // We first look for SMTK as run from the build tree.
  bool found = this->addPathToBuildTree(dir, module);

  // If we don't find it, then we look for the module as an installed project.
  if (!found)
  {
    found = this->addPathToInstalledModule(dir, module);
  }

  // If we don't find it, then we look for the module as a packaged project.
  if (!found)
  {
    found = this->addPathToPackagedModule(dir, module);
  }

  return found;
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
          << "    import " << module << "\n";
  if (Environment::hasVariable("SMTK_PYTHON_MODULE_LOAD_VERBOSE"))
  {
    testCmd << "except ImportError as error:\n"
            << "    print(str(error))\n";
  }
  else
  {
    testCmd << "except ImportError:\n";
  }
  testCmd << "    found = False";
  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  found = locals["found"].cast<bool>();
  return found;
}

bool PythonInterpreter::addPathToPackagedModule(
  const std::string& packageDir,
  const std::string& module)
{
  // If <module> is run out of a package, we add its directory to the PYTHONPATH.
  // The path to the module is specific to each platform.

#ifdef __APPLE__
  boost::filesystem::path inputPath = boost::filesystem::path(packageDir);
  boost::filesystem::path absPath = boost::filesystem::canonical(inputPath);
  boost::filesystem::path pythonPath = absPath / "Python";
#endif

#ifdef __linux__
  boost::filesystem::path inputPath = boost::filesystem::path(packageDir);
  boost::filesystem::path absPath = boost::filesystem::canonical(inputPath);
  boost::filesystem::path pythonPath = absPath.parent_path().parent_path() / python_moduledir;
#endif

#ifdef _WIN32
  boost::filesystem::path pythonPath = boost::filesystem::path(packageDir) / python_moduledir;
#endif

  boost::filesystem::path bundledPyInit = pythonPath / module / "__init__.py";
  if (boost::filesystem::is_regular_file(bundledPyInit))
  {
    this->addToPythonPath(pythonPath.string());
    return true;
  }
  return false;
}

bool PythonInterpreter::addPathToInstalledModule(
  const std::string& installDir,
  const std::string& module)
{
  // If <module> is run out of the install tree, we expect that the directory
  // that contains its libraries also contains a directory called
  // "python<PY_MAJOR_VERSION>.<PY_MINOR_VERSION>"/site-packages/<module>", so
  // we attempt to add this directory to the PYTHONPATH.

  boost::filesystem::path pythonPath = boost::filesystem::path(installDir) / python_moduledir;
  boost::filesystem::path installedPyInit = pythonPath / module / "__init__.py";

  if (boost::filesystem::is_regular_file(installedPyInit))
  {
    this->addToPythonPath(pythonPath.string());
    return true;
  }
  return false;
}

bool PythonInterpreter::addPathToBuildTree(
  const std::string& buildTreePath,
  const std::string& module)
{
  // If <module> is run out of the build tree, we expect the directory that
  // contains its libraries to reside one level below the build directory,
  // so we attempt to add the build directory to the PYTHONPATH.

  boost::filesystem::path pythonPath = boost::filesystem::path(buildTreePath) / python_moduledir;
  boost::filesystem::path buildTreePyInit = pythonPath / module / "__init__.py";

  if (boost::filesystem::is_regular_file(buildTreePyInit))
  {
    this->addToPythonPath(pythonPath.string());
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
  const std::string& fName,
  const std::string& moduleName)
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
          << "    import sys, types, importlib.machinery\n"
          << "    loader = importlib.machinery.SourceFileLoader('" << moduleName << "', '"
          << fileName << "')\n"
          << "    mod = types.ModuleType(loader.name)\n"
          << "    loader.exec_module(mod)\n"
          << "    sys.modules['" << moduleName << "'] = mod\n"
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3 && PY_MINOR_VERSION <= 4
          << "    from importlib.machinery import SourceFileLoader\n"
          << "    tmp = SourceFileLoader('" << moduleName << "', '" << fileName
          << "').load_module()\n"
#else /* PY_MAJOR_VERSION == 2 */
          << "    import imp\n"
          << "    tmp = imp.load_source('" << moduleName << "', '" << fileName << "')\n"
#endif
          << "except Exception as e:\n"
          << "    print(e)\n"
          << "    loaded = False";

  pybind11::exec(testCmd.str().c_str(), pybind11::globals(), locals);

  loaded = locals["loaded"].cast<bool>();

  this->dontWriteByteCode(dontWriteByteCode);

  return loaded;
}
} // namespace common
} // namespace smtk
