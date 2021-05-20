//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Tags_h
#define pybind_smtk_project_Tags_h

#include <pybind11/pybind11.h>

#include "smtk/project/Tags.h"

namespace py = pybind11;

inline py::class_< smtk::project::IdTag > pybind11_init_smtk_project_IdTag(py::module &m)
{
  py::class_< smtk::project::IdTag > instance(m, "IdTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::IdTag const &>())
    .def("deepcopy", (smtk::project::IdTag & (smtk::project::IdTag::*)(::smtk::project::IdTag const &)) &smtk::project::IdTag::operator=)
    ;
  return instance;
}

inline py::class_< smtk::project::IndexTag > pybind11_init_smtk_project_IndexTag(py::module &m)
{
  py::class_< smtk::project::IndexTag > instance(m, "IndexTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::IndexTag const &>())
    .def("deepcopy", (smtk::project::IndexTag & (smtk::project::IndexTag::*)(::smtk::project::IndexTag const &)) &smtk::project::IndexTag::operator=)
    ;
  return instance;
}

inline py::class_< smtk::project::LocationTag > pybind11_init_smtk_project_LocationTag(py::module &m)
{
  py::class_< smtk::project::LocationTag > instance(m, "LocationTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::LocationTag const &>())
    .def("deepcopy", (smtk::project::LocationTag & (smtk::project::LocationTag::*)(::smtk::project::LocationTag const &)) &smtk::project::LocationTag::operator=)
    ;
  return instance;
}

inline py::class_< smtk::project::NameTag > pybind11_init_smtk_project_NameTag(py::module &m)
{
  py::class_< smtk::project::NameTag > instance(m, "NameTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::NameTag const &>())
    .def("deepcopy", (smtk::project::NameTag & (smtk::project::NameTag::*)(::smtk::project::NameTag const &)) &smtk::project::NameTag::operator=)
    ;
  return instance;
}

inline py::class_< smtk::project::RoleTag > pybind11_init_smtk_project_RoleTag(py::module &m)
{
  py::class_< smtk::project::RoleTag > instance(m, "RoleTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::project::RoleTag const &>())
    .def("deepcopy", (smtk::project::RoleTag & (smtk::project::RoleTag::*)(::smtk::project::RoleTag const &)) &smtk::project::RoleTag::operator=)
    ;
  return instance;
}

#endif
