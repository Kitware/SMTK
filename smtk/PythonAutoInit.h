//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_PythonAutoInit_h
#define __smtk_PythonAutoInit_h
/*! \file */

// This file contains macros used to initialize components of SMTK
// that may defined in separate libraries but which need to be
// exposed to SMTK's core components.
//
// See smtk/model/Session.h and its subclasses for an example of how
// these macros are used to register components with the model Manager.

#include "smtk/common/PythonInterpreter.h"

#include "pybind11/pybind11.h"

#include <iostream>

#define smtkPythonInitMacro(C, ModuleName)                                                         \
  static struct C##_PythonComponentInit                                                            \
  {                                                                                                \
    /* Call <mod>_AutoInit_Construct during initialization.  */                                    \
    C##_PythonComponentInit()                                                                      \
    {                                                                                              \
      smtk::common::PythonInterpreter::instance().initialize();                                    \
      pybind11::module mod = pybind11::module::import(#ModuleName);                                \
    }                                                                                              \
    /* Call <mod>_AutoInit_Destruct during finalization.  */                                       \
    ~C##_PythonComponentInit() {}                                                                  \
  } C##_PythonComponentInit_Instance;

#endif // __smtk_PythonAutoInit_h
