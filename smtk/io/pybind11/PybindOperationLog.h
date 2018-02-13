//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_OperationLog_h
#define pybind_smtk_io_OperationLog_h

#include <pybind11/pybind11.h>

#include "smtk/io/OperationLog.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::OperationLog > pybind11_init_smtk_io_OperationLog(py::module &m)
{
  PySharedPtrClass< smtk::io::OperationLog > instance(m, "OperationLog");
  instance
    .def(py::init<smtk::model::ManagerPtr>())
    ;
  return instance;
}

#endif
