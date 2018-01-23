//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Container_h
#define pybind_smtk_resource_Container_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Container.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Resource.h"

namespace py = pybind11;

py::class_< smtk::resource::IdTag > pybind11_init_smtk_resource_IdTag(py::module &m)
{
  py::class_< smtk::resource::IdTag > instance(m, "IdTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::IdTag const &>())
    .def("deepcopy", (smtk::resource::IdTag & (smtk::resource::IdTag::*)(::smtk::resource::IdTag const &)) &smtk::resource::IdTag::operator=)
    ;
  return instance;
}

py::class_< smtk::resource::IndexTag > pybind11_init_smtk_resource_IndexTag(py::module &m)
{
  py::class_< smtk::resource::IndexTag > instance(m, "IndexTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::IndexTag const &>())
    .def("deepcopy", (smtk::resource::IndexTag & (smtk::resource::IndexTag::*)(::smtk::resource::IndexTag const &)) &smtk::resource::IndexTag::operator=)
    ;
  return instance;
}

py::class_< smtk::resource::LocationTag > pybind11_init_smtk_resource_LocationTag(py::module &m)
{
  py::class_< smtk::resource::LocationTag > instance(m, "LocationTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::LocationTag const &>())
    .def("deepcopy", (smtk::resource::LocationTag & (smtk::resource::LocationTag::*)(::smtk::resource::LocationTag const &)) &smtk::resource::LocationTag::operator=)
    ;
  return instance;
}

py::class_< smtk::resource::NameTag > pybind11_init_smtk_resource_NameTag(py::module &m)
{
  py::class_< smtk::resource::NameTag > instance(m, "NameTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::NameTag const &>())
    .def("deepcopy", (smtk::resource::NameTag & (smtk::resource::NameTag::*)(::smtk::resource::NameTag const &)) &smtk::resource::NameTag::operator=)
    ;
  return instance;
}

void pybind11_init_smtk_resource_detail_id(py::module &m)
{
  m.def("id", &smtk::resource::detail::id, "", py::arg("r"));
}

void pybind11_init_smtk_resource_detail_index(py::module &m)
{
  m.def("index", &smtk::resource::detail::index, "", py::arg("r"));
}

void pybind11_init_smtk_resource_detail_location(py::module &m)
{
  m.def("location", &smtk::resource::detail::location, "", py::arg("r"));
}

#endif
