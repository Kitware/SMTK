//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_PythonInterpreter_h
#define __smtk_PythonInterpreter_h

#include "smtk/CoreExports.h"

#include <string>
#include <vector>

namespace smtk
{
namespace common
{

/**\brief A singleton class for encapsulating the embedded interpreter.

   Actions that invoke pybind11's embedded interpreter (such as python-based
   operators) must be be called while the interpreter is initialized. This
   singleton ensures that the interpreter is initialized/finalized at static
   initialization/destruction.

   NB: This class should not be wrapped.
  */
class SMTKCORE_EXPORT PythonInterpreter
{
public:
  static PythonInterpreter& instance();

  // Check if python is initialized.
  bool isInitialized() const;

  // Initialize the embedded python, with additional logic to add SMTK to the
  // PYTHONPATH.
  void initialize();

  // Finalize the embedded python.
  void finalize();

  // Return a list of the directories in the embedded python's PYTHONPATH
  std::vector<std::string> pythonPath();

  // Given a string <paths> containing a <separator>-separated list of paths,
  // add each path to the embedded python's PYTHONPATH.
  bool addToPythonPath(const std::string& paths, std::string separator = ",");

  // Check if <module> can be loaded by the python interpereter.
  bool canFindModule(const std::string& module) const;

  // Given the packaging semantics used in SMTK, VTK, ParaView, etc., locate
  // <module> and add it to the embedded python's PYTHONPATH, and return a
  // success flag.
  bool addPathToPackagedModule(const std::string& libPackageDir, const std::string& module);

  // Given the install semantics used in SMTK, VTK, ParaView, etc., locate
  // <module> and add it to the embedded python's PYTHONPATH, and return a
  // success flag.
  bool addPathToInstalledModule(const std::string& libInstallDir, const std::string& module);

  // Given the build semantics used in SMTK, VTK, ParaView, etc., locate
  // <module> and add it to the embedded python's PYTHONPATH, and return a
  // success flag.
  bool addPathToBuildTree(const std::string& buildTreePath, const std::string& module);

  // Returns true if the embedded python session has been initialized.
  bool isEmbedded() const { return m_embedded; }

  // Load a .py file given its absolute path.
  bool loadPythonSourceFile(const std::string& fileName);
  bool loadPythonSourceFile(const std::string& fileName, const std::string& moduleName);

private:
  PythonInterpreter();
  virtual ~PythonInterpreter();

  static PythonInterpreter m_instance;

  bool m_embedded;
};
}
}

#endif
