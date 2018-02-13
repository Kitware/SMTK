//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_remus_MeshOperation_h
#define pybind_smtk_extension_remus_MeshOperation_h

#include <pybind11/pybind11.h>

#include "smtk/extension/remus/MeshOperation.h"

PySharedPtrClass< smtk::model::MeshOperation, smtk::operation::Operation > pybind11_init_smtk_extension_remus_MeshOperation(py::module &m)
{
  PySharedPtrClass< smtk::model::MeshOperation, smtk::operation::Operation > instance(m, "MeshOperation");
  instance
    .def("classname", &smtk::model::MeshOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::model::MeshOperation> (*)()) &smtk::model::MeshOperation::create)
    .def_static("create", (std::shared_ptr<smtk::model::MeshOperation> (*)(::std::shared_ptr<smtk::model::MeshOperation> &)) &smtk::model::MeshOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::model::MeshOperation> (smtk::model::MeshOperation::*)()) &smtk::model::MeshOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::MeshOperation> (smtk::model::MeshOperation::*)() const) &smtk::model::MeshOperator::shared_from_this)
    .def("name", &smtk::model::MeshOperation::name)
    .def("className", &smtk::model::MeshOperation::className)
    .def("ableToOperate", &smtk::model::MeshOperation::ableToOperate)
    ;
  return instance;
}

#endif
