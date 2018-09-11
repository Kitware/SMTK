//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_WriteResource_h
#define pybind_smtk_session_discrete_operators_WriteResource_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/WriteResource.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::WriteResource, smtk::operation::Operation > pybind11_init_smtk_session_discrete_WriteResource(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::WriteResource, smtk::operation::Operation > instance(m, "WriteResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::WriteResource> (*)()) &smtk::session::discrete::WriteResource::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::WriteResource> (*)(::std::shared_ptr<smtk::session::discrete::WriteResource> &)) &smtk::session::discrete::WriteOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::WriteResource> (smtk::session::discrete::WriteResource::*)()) &smtk::session::discrete::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::WriteResource> (smtk::session::discrete::WriteResource::*)() const) &smtk::session::discrete::WriteOperator::shared_from_this)
    .def("name", &smtk::session::discrete::WriteResource::name)
    .def("ableToOperate", &smtk::session::discrete::WriteResource::ableToOperate)
    ;

  m.def("write", (bool (*)(const std::resource::ResourcePtr)) &smtk::session::discrete::writeResource, "", py::arg("resource"));

  return instance;
}

#endif
