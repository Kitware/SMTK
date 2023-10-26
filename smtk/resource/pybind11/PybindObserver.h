//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Observer_h
#define pybind_smtk_resource_Observer_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Attribute.h"

#include "smtk/resource/Observer.h"

namespace py = pybind11;

inline void pybind11_init_smtk_resource_EventType(py::module &m)
{
  py::enum_<smtk::resource::EventType>(m, "EventType")
    .value("ADDED", smtk::resource::EventType::ADDED)
    .value("REMOVED", smtk::resource::EventType::REMOVED)
    .export_values();
}

inline py::class_< smtk::resource::Observers > pybind11_init_smtk_resource_Observers(py::module &m)
{
  py::class_< smtk::resource::Observers > instance(m, "Observers");
  instance
    .def(py::init<>())
    .def("__call__", [](smtk::resource::Observers& observers, const smtk::resource::Resource& rsrc, ::smtk::resource::EventType eventType) { return observers(rsrc, eventType); })
    .def("__len__", &smtk::resource::Observers::size)
    .def("erase", (std::size_t (smtk::resource::Observers::*)(smtk::resource::Observers::Key&)) &smtk::resource::Observers::erase)
    .def("insert", (smtk::resource::Observers::Key (smtk::resource::Observers::*)(smtk::resource::Observer, std::string)) &smtk::resource::Observers::insert, pybind11::keep_alive<1, 2>())
    .def("insert", (smtk::resource::Observers::Key (smtk::resource::Observers::*)(smtk::resource::Observer, smtk::resource::Observers::Priority, bool, std::string)) &smtk::resource::Observers::insert, pybind11::keep_alive<1, 2>())
    ;
  py::class_< smtk::resource::Observers::Key >(instance, "Key")
    .def(py::init<>())
    .def("assigned", &smtk::resource::Observers::Key::assigned)
    ;
  return instance;
}

#endif
