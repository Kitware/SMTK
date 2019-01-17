//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_simulation_UserData_h
#define pybind_smtk_simulation_UserData_h

#include <pybind11/pybind11.h>

#include "smtk/simulation/UserData.h"

namespace py = pybind11;

PySharedPtrClass< smtk::simulation::UserData > pybind11_init_smtk_simulation_UserData(py::module &m)
{
  PySharedPtrClass< smtk::simulation::UserData > instance(m, "UserData");
  instance
    .def(py::init<::smtk::simulation::UserData const &>())
    .def("deepcopy", (smtk::simulation::UserData & (smtk::simulation::UserData::*)(::smtk::simulation::UserData const &)) &smtk::simulation::UserData::operator=)
    .def_static("New", &smtk::simulation::UserData::New)
    ;
  return instance;
}

PySharedPtrClass< smtk::simulation::UserDataInt, smtk::simulation::UserData > pybind11_init_smtk_simulation_UserDataInt(py::module &m)
{
  PySharedPtrClass< smtk::simulation::UserDataInt, smtk::simulation::UserData  > instance(m, "UserDataInt");
  instance
    .def(py::init<::smtk::simulation::UserDataInt const &>())
    .def("deepcopy", (smtk::simulation::UserDataInt & (smtk::simulation::UserDataInt::*)(::smtk::simulation::UserDataInt const &)) &smtk::simulation::UserDataInt::operator=)
    .def_static("New", &smtk::simulation::UserDataInt::New)
    .def("value", &smtk::simulation::UserDataInt::value)
    .def("setValue", &smtk::simulation::UserDataInt::setValue, py::arg("value"))
    ;
  return instance;
}

PySharedPtrClass< smtk::simulation::UserDataDouble, smtk::simulation::UserData > pybind11_init_smtk_simulation_UserDataDouble(py::module &m)
{
  PySharedPtrClass< smtk::simulation::UserDataDouble, smtk::simulation::UserData  > instance(m, "UserDataDouble");
  instance
    .def(py::init<::smtk::simulation::UserDataDouble const &>())
    .def("deepcopy", (smtk::simulation::UserDataDouble & (smtk::simulation::UserDataDouble::*)(::smtk::simulation::UserDataDouble const &)) &smtk::simulation::UserDataDouble::operator=)
    .def_static("New", &smtk::simulation::UserDataDouble::New)
    .def("value", &smtk::simulation::UserDataDouble::value)
    .def("setValue", &smtk::simulation::UserDataDouble::setValue, py::arg("value"))
    ;
  return instance;
}

PySharedPtrClass< smtk::simulation::UserDataString, smtk::simulation::UserData > pybind11_init_smtk_simulation_UserDataString(py::module &m)
{
  PySharedPtrClass< smtk::simulation::UserDataString, smtk::simulation::UserData  > instance(m, "UserDataString");
  instance
    .def(py::init<::smtk::simulation::UserDataString const &>())
    .def("deepcopy", (smtk::simulation::UserDataString & (smtk::simulation::UserDataString::*)(::smtk::simulation::UserDataString const &)) &smtk::simulation::UserDataString::operator=)
    .def_static("New", &smtk::simulation::UserDataString::New)
    .def("value", &smtk::simulation::UserDataString::value)
    .def("setValue", &smtk::simulation::UserDataString::setValue, py::arg("value"))
    ;
  return instance;
}

#endif
