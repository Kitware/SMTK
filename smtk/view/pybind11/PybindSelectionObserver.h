//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_view_SelectionObserver_h
#define pybind_smtk_view_SelectionObserver_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Attribute.h"

#include "smtk/view/SelectionObserver.h"

namespace py = pybind11;

inline py::class_< smtk::view::SelectionObservers > pybind11_init_smtk_view_SelectionObservers(py::module &m)
{
  py::class_< smtk::view::SelectionObservers > instance(m, "SelectionObservers");
  instance
    .def(py::init<>())
    .def("__call__", [](smtk::view::SelectionObservers& observers, const std::string& str, smtk::view::SelectionPtr sel) { observers(str, sel); })
    .def("__len__", &smtk::view::SelectionObservers::size)
    .def("erase", (std::size_t (smtk::view::SelectionObservers::*)(smtk::view::SelectionObservers::Key&)) &smtk::view::SelectionObservers::erase)
    .def("insert", (smtk::view::SelectionObservers::Key (smtk::view::SelectionObservers::*)(smtk::view::SelectionObserver, std::string)) &smtk::view::SelectionObservers::insert, pybind11::keep_alive<1, 2>())
    .def("insert", (smtk::view::SelectionObservers::Key (smtk::view::SelectionObservers::*)(smtk::view::SelectionObserver, smtk::view::SelectionObservers::Priority, bool, std::string)) &smtk::view::SelectionObservers::insert, pybind11::keep_alive<1, 2>())
    .def("find", &smtk::view::SelectionObservers::find)
    ;
  py::class_< smtk::view::SelectionObservers::Key >(instance, "Key")
    .def(py::init<>())
    .def("assigned", &smtk::view::SelectionObservers::Key::assigned)
    ;
  return instance;
}

#endif
