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

  bool isInitialized() const;

  void initialize();
  void finalize();

private:
  PythonInterpreter();
  virtual ~PythonInterpreter();

  static PythonInterpreter m_instance;
};
}
}

#endif
