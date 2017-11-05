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

static PyMethodDef methods[] = { { NULL, NULL, 0, NULL } };

PyMODINIT_FUNC initsmtkRemusExtPy()
{
#if (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3) || PY_MAJOR_VERSION > 3
  static struct PyModuleDef remusModule = {
    PyModuleDef_HEAD_INIT, "smtkRemusExtPy", /* name of module */
    nullptr,                                 /* module documentation */
    -1,     /* size of per-interpreter module state or -1 if in globals */
    methods /* method pointers from above */
  };
  PyModule_Create(&remusModule);
#else
  Py_InitModule("smtkRemusExtPy", methods);
#endif
  (void)smtk_remus_mesh_operator_ComponentInit_Instance;
}

} // extern "C"
