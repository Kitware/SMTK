//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_discrete_Session_h
#define pybind_smtk_session_discrete_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/discrete/Session.h"

#include "smtk/model/Session.h"

#include "vtkUnsignedIntArray.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::discrete::Session, smtk::model::Session > pybind11_init_smtk_session_discrete_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::discrete::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::session::discrete::Session const &>())
    .def("deepcopy", (smtk::session::discrete::Session & (smtk::session::discrete::Session::*)(::smtk::session::discrete::Session const &)) &smtk::session::discrete::Session::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::Session> (*)()) &smtk::session::discrete::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::discrete::Session> (*)(::std::shared_ptr<smtk::session::discrete::Session> &)) &smtk::session::discrete::Session::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::discrete::Session> (smtk::session::discrete::Session::*)()) &smtk::session::discrete::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::discrete::Session> (smtk::session::discrete::Session::*)() const) &smtk::session::discrete::Session::shared_from_this)
    .def_static("staticClassName", &smtk::session::discrete::Session::staticClassName)
    .def("name", &smtk::session::discrete::Session::name)
    .def("allSupportedInformation", &smtk::session::discrete::Session::allSupportedInformation)
    .def("assignUUIDs", &smtk::session::discrete::Session::assignUUIDs, py::arg("ents"), py::arg("uuidArray"))
    .def("retrieveUUIDs", &smtk::session::discrete::Session::retrieveUUIDs, py::arg("model"), py::arg("ents"))
    .def("ExportEntitiesToFileOfNameAndType", &smtk::session::discrete::Session::ExportEntitiesToFileOfNameAndType, py::arg("entities"), py::arg("filename"), py::arg("filetype"))
    .def("findModelEntity", &smtk::session::discrete::Session::findModelEntity, py::arg("uid"))
    ;
  return instance;
}

#endif
