//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_FillOutAttributes_h
#define pybind_smtk_task_FillOutAttributes_h

#include <pybind11/pybind11.h>

#include "smtk/task/FillOutAttributes.h"

#include "smtk/common/Visit.h"
#include "smtk/task/Task.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::FillOutAttributes, smtk::task::Task > pybind11_init_smtk_task_FillOutAttributes(py::module &m)
{
  PySharedPtrClass< smtk::task::FillOutAttributes, smtk::task::Task > instance(m, "FillOutAttributes");
  instance
    .def("configure", [](smtk::task::Task& task, const std::string& jsonConfig)
      {
        auto config = nlohmann::json::parse(jsonConfig);
        task.configure(config);
      }, py::arg("config"))
    .def_static("create", (std::shared_ptr<smtk::task::FillOutAttributes> (*)()) &smtk::task::FillOutAttributes::create)
    .def_static("create", (std::shared_ptr<smtk::task::FillOutAttributes> (*)(::std::shared_ptr<smtk::task::FillOutAttributes> &)) &smtk::task::FillOutAttributes::create, py::arg("ref"))
    .def("typeName", &smtk::task::FillOutAttributes::typeName)
    .def("visitAttributeSets", (smtk::common::Visit (smtk::task::FillOutAttributes::*)(::smtk::task::FillOutAttributes::ConstAttributeSetVisitor) const) &smtk::task::FillOutAttributes::visitAttributeSets, py::arg("visitor"))
    .def("visitAttributeSets", (smtk::common::Visit (smtk::task::FillOutAttributes::*)(::smtk::task::FillOutAttributes::AttributeSetVisitor)) &smtk::task::FillOutAttributes::visitAttributeSets, py::arg("visitor"))

    ;
  py::class_< smtk::task::FillOutAttributes::AttributeSet >(instance, "AttributeSet")
    .def(py::init<>())
    .def(py::init<::smtk::task::FillOutAttributes::AttributeSet const &>())
    .def("deepcopy", (smtk::task::FillOutAttributes::AttributeSet & (smtk::task::FillOutAttributes::AttributeSet::*)(::smtk::task::FillOutAttributes::AttributeSet const &)) &smtk::task::FillOutAttributes::AttributeSet::operator=)
    .def_readwrite("m_definitions", &smtk::task::FillOutAttributes::AttributeSet::m_definitions)
    .def_readwrite("m_resources", &smtk::task::FillOutAttributes::AttributeSet::m_resources)
    .def_readwrite("m_role", &smtk::task::FillOutAttributes::AttributeSet::m_role)
    ;
  py::class_< smtk::task::FillOutAttributes::ResourceAttributes >(instance, "ResourceAttributes")
    .def(py::init<>())
    .def(py::init<::smtk::task::FillOutAttributes::ResourceAttributes const &>())
    .def("deepcopy", (smtk::task::FillOutAttributes::ResourceAttributes & (smtk::task::FillOutAttributes::ResourceAttributes::*)(::smtk::task::FillOutAttributes::ResourceAttributes const &)) &smtk::task::FillOutAttributes::ResourceAttributes::operator=)
    .def_readwrite("m_invalid", &smtk::task::FillOutAttributes::ResourceAttributes::m_invalid)
    .def_readwrite("m_valid", &smtk::task::FillOutAttributes::ResourceAttributes::m_valid)
    ;
  return instance;
}

#endif
