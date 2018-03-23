//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_WriteResource_h
#define pybind_smtk_bridge_discrete_operators_WriteResource_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/WriteResource.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::WriteResource, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_WriteResource(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::WriteResource, smtk::operation::Operation > instance(m, "WriteResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteResource> (*)()) &smtk::bridge::discrete::WriteResource::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteResource> (*)(::std::shared_ptr<smtk::bridge::discrete::WriteResource> &)) &smtk::bridge::discrete::WriteOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::WriteResource> (smtk::bridge::discrete::WriteResource::*)()) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::WriteResource> (smtk::bridge::discrete::WriteResource::*)() const) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::WriteResource::name)
    .def("ableToOperate", &smtk::bridge::discrete::WriteResource::ableToOperate)
    ;

  m.def("write", (bool (*)(const std::resource::ResourcePtr)) &smtk::bridge::discrete::writeResource, "", py::arg("resource"));

  return instance;
}

#endif
