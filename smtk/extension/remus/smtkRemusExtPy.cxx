//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <Python.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/AutoInit.h"

/**!\file smtkRemusExtPy.cxx
  *\brief Provide a library to force registration of the mesh operator from Python.
  *
  * From python, just "import smtkRemusExtPy" before you import smtk;
  * then, the mesh operator should be available to any session.
  */

smtkComponentInitMacro(smtk_remus_mesh_operator);

extern "C" {

static PyMethodDef methods[] = {
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initsmtkRemusExtPy()
{
  Py_InitModule("smtkRemusExtPy", methods);
  (void)smtk_remus_mesh_operator_ComponentInit_Instance;
}

} // extern "C"
