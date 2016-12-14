//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_SessionIO_h
#define pybind_smtk_model_SessionIO_h

#include <pybind11/pybind11.h>

#include "smtk/model/SessionIO.h"

namespace py = pybind11;

py::class_< smtk::model::SessionIO > pybind11_init_smtk_model_SessionIO(py::module &m)
{
  py::class_< smtk::model::SessionIO > instance(m, "SessionIO");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::SessionIO const &>())
    .def("deepcopy", (smtk::model::SessionIO & (smtk::model::SessionIO::*)(::smtk::model::SessionIO const &)) &smtk::model::SessionIO::operator=)
    .def("classname", &smtk::model::SessionIO::classname)
    .def("referencePath", &smtk::model::SessionIO::referencePath)
    .def("setReferencePath", &smtk::model::SessionIO::setReferencePath, py::arg("p"))
    ;
  return instance;
}

#endif
