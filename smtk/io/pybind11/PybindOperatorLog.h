//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_OperatorLog_h
#define pybind_smtk_io_OperatorLog_h

#include <pybind11/pybind11.h>

#include "smtk/io/OperatorLog.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::OperatorLog > pybind11_init_smtk_io_OperatorLog(py::module &m)
{
  PySharedPtrClass< smtk::io::OperatorLog > instance(m, "OperatorLog");
  instance
    .def(py::init<smtk::model::ManagerPtr>())
    ;
  return instance;
}

#endif
