//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityIterator_h
#define pybind_smtk_model_EntityIterator_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityIterator.h"

#include "smtk/model/EntityRef.h"

namespace py = pybind11;

void pybind11_init_smtk_model_IteratorStyle(py::module &m)
{
  py::enum_<smtk::model::IteratorStyle>(m, "IteratorStyle")
    .value("ITERATE_BARE", smtk::model::IteratorStyle::ITERATE_BARE)
    .value("ITERATE_CHILDREN", smtk::model::IteratorStyle::ITERATE_CHILDREN)
    .value("ITERATE_MODELS", smtk::model::IteratorStyle::ITERATE_MODELS)
    .export_values();
}

py::class_< smtk::model::EntityIterator > pybind11_init_smtk_model_EntityIterator(py::module &m)
{
  py::class_< smtk::model::EntityIterator > instance(m, "EntityIterator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityIterator const &>())
    .def("__mul__", (smtk::model::EntityRef (smtk::model::EntityIterator::*)() const) &smtk::model::EntityIterator::operator*)
    .def("deepcopy", (smtk::model::EntityIterator & (smtk::model::EntityIterator::*)(::smtk::model::EntityIterator const &)) &smtk::model::EntityIterator::operator=)
    .def("advance", &smtk::model::EntityIterator::advance)
    .def("begin", &smtk::model::EntityIterator::begin)
    .def("current", &smtk::model::EntityIterator::current)
    // .def("end", &smtk::model::EntityIterator::end)
    .def("isAtEnd", &smtk::model::EntityIterator::isAtEnd)
    .def("traverse", (void (smtk::model::EntityIterator::*)(::smtk::model::EntityRef const &)) &smtk::model::EntityIterator::traverse, py::arg("x"))
    .def("traverse", (void (smtk::model::EntityIterator::*)(::smtk::model::EntityRef const &, ::smtk::model::IteratorStyle)) &smtk::model::EntityIterator::traverse, py::arg("x"), py::arg("style"))
    ;
  return instance;
}

#endif
