//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_Session_h
#define pybind_smtk_bridge_discrete_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/Session.h"

#include "vtkUnsignedIntArray.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::Session, smtk::model::Session > pybind11_init_smtk_bridge_discrete_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::bridge::discrete::Session const &>())
    .def("deepcopy", (smtk::bridge::discrete::Session & (smtk::bridge::discrete::Session::*)(::smtk::bridge::discrete::Session const &)) &smtk::bridge::discrete::Session::operator=)
    .def("classname", &smtk::bridge::discrete::Session::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::Session> (*)()) &smtk::bridge::discrete::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::Session> (*)(::std::shared_ptr<smtk::bridge::discrete::Session> &)) &smtk::bridge::discrete::Session::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::Session> (smtk::bridge::discrete::Session::*)()) &smtk::bridge::discrete::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::Session> (smtk::bridge::discrete::Session::*)() const) &smtk::bridge::discrete::Session::shared_from_this)
    .def_static("staticClassName", &smtk::bridge::discrete::Session::staticClassName)
    .def("name", &smtk::bridge::discrete::Session::name)
    .def("className", &smtk::bridge::discrete::Session::className)
    .def("allSupportedInformation", &smtk::bridge::discrete::Session::allSupportedInformation)
    .def("assignUUIDs", &smtk::bridge::discrete::Session::assignUUIDs, py::arg("ents"), py::arg("uuidArray"))
    .def("retrieveUUIDs", &smtk::bridge::discrete::Session::retrieveUUIDs, py::arg("model"), py::arg("ents"))
    .def("ExportEntitiesToFileOfNameAndType", &smtk::bridge::discrete::Session::ExportEntitiesToFileOfNameAndType, py::arg("entities"), py::arg("filename"), py::arg("filetype"))
    .def("findModelEntity", &smtk::bridge::discrete::Session::findModelEntity, py::arg("uid"))
    ;
  return instance;
}

#endif
