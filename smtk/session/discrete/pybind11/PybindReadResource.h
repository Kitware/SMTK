//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_operators_ReadResource_h
#define pybind_smtk_session_discrete_operators_ReadResource_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/operators/ReadResource.h"


namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::ReadResource, smtk::operation::Operation > pybind11_init_smtk_session_discrete_ReadResource(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::ReadResource, smtk::operation::Operation > instance(m, "ReadResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::discrete::ReadResource> (*)()) &smtk::session::discrete::ReadResource::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::ReadResource> (*)(::std::shared_ptr<smtk::session::discrete::ReadResource> &)) &smtk::session::discrete::ReadOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::ReadResource> (smtk::session::discrete::ReadResource::*)()) &smtk::session::discrete::ReadOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::ReadResource> (smtk::session::discrete::ReadResource::*)() const) &smtk::session::discrete::ReadOperator::shared_from_this)
    .def("name", &smtk::session::discrete::ReadResource::name)
    .def("ableToOperate", &smtk::session::discrete::ReadResource::ableToOperate)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &)) &smtk::session::discrete::readResource, "", py::arg("filePath"));

  return instance;
}

#endif
