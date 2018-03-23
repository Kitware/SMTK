//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_ReadResource_h
#define pybind_smtk_bridge_discrete_operators_ReadResource_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/ReadResource.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::ReadResource, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_ReadResource(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::ReadResource, smtk::operation::Operation > instance(m, "ReadResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ReadResource> (*)()) &smtk::bridge::discrete::ReadResource::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ReadResource> (*)(::std::shared_ptr<smtk::bridge::discrete::ReadResource> &)) &smtk::bridge::discrete::ReadOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::ReadResource> (smtk::bridge::discrete::ReadResource::*)()) &smtk::bridge::discrete::ReadOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::ReadResource> (smtk::bridge::discrete::ReadResource::*)() const) &smtk::bridge::discrete::ReadOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::ReadResource::name)
    .def("ableToOperate", &smtk::bridge::discrete::ReadResource::ableToOperate)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::bridge::discrete::readResource, "", py::arg("filePath"));

  return instance;
}

#endif
