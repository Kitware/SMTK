//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_remus_MeshOperator_h
#define pybind_smtk_extension_remus_MeshOperator_h

#include <pybind11/pybind11.h>

#include "smtk/extension/remus/MeshOperator.h"

PySharedPtrClass< smtk::model::MeshOperator, smtk::model::Operator > pybind11_init_smtk_extension_remus_MeshOperator(py::module &m)
{
  PySharedPtrClass< smtk::model::MeshOperator, smtk::model::Operator > instance(m, "MeshOperator");
  instance
    .def("classname", &smtk::model::MeshOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::model::MeshOperator> (*)()) &smtk::model::MeshOperator::create)
    .def_static("create", (std::shared_ptr<smtk::model::MeshOperator> (*)(::std::shared_ptr<smtk::model::MeshOperator> &)) &smtk::model::MeshOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::model::MeshOperator> (smtk::model::MeshOperator::*)()) &smtk::model::MeshOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::model::MeshOperator> (smtk::model::MeshOperator::*)() const) &smtk::model::MeshOperator::shared_from_this)
    .def("name", &smtk::model::MeshOperator::name)
    .def("className", &smtk::model::MeshOperator::className)
    .def_static("baseCreate", &smtk::model::MeshOperator::baseCreate)
    .def("ableToOperate", &smtk::model::MeshOperator::ableToOperate)
    ;
  return instance;
}

#endif
