//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_AutoInit_h
#define smtk_AutoInit_h
/*! \file */

// This file contains macros used to initialize components of SMTK
// that may defined in separate libraries but which need to be
// exposed to SMTK's core components.
//
// See smtk/model/Session.h and its subclasses for an example of how
// these macros are used to register components with the model Manager.

#define smtkAutoInitComponentMacro(C)                                                              \
  void C##_AutoInit_Construct();                                                                   \
  void C##_AutoInit_Destruct();
#define smtkAutoInitConstructMacro(C) C##_AutoInit_Construct();
#define smtkAutoInitDestructMacro(C) C##_AutoInit_Destruct();

/**\brief Register an SMTK component for use.
  *
  * Initialize the named SMTK component (such as a modeling kernel), ensuring
  * it is correctly registered and unregistered. This call must be made in
  * global scope in the translation unit of your executable (which can include
  * code built into a shared library linked to your executable, but will not
  * work as expected in code linked to your executable as part of a static
  * library).
  *
  * @code{.cpp}
  * #include "vtkAutoInit.h"
  * smtkComponentInitMacro(smtk_cgm_session);
  * @endcode
  *
  * If included in the global scope, the above snippet will ensure the
  * global function smtk_cgm_session_AutoInit_Construct is called during
  * dynamic C++ initialization and and the global function
  * smtk_cgm_session_AutoInit_Destruct is called during finalization.
  */
#define smtkComponentInitMacro(C)                                                                  \
  smtkAutoInitComponentMacro(C) static struct C##_ComponentInit                                    \
  {                                                                                                \
    /* Call <mod>_AutoInit_Construct during initialization.  */                                    \
    C##_ComponentInit()                                                                            \
    {                                                                                              \
      smtkAutoInitConstructMacro(C)                                                                \
    }                                                                                              \
    /* Call <mod>_AutoInit_Destruct during finalization.  */                                       \
    ~C##_ComponentInit()                                                                           \
    {                                                                                              \
      smtkAutoInitDestructMacro(C)                                                                 \
    }                                                                                              \
  } C##_ComponentInit_Instance;

#endif // smtk_AutoInit_h
