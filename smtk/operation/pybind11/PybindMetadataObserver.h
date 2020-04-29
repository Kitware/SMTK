//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_MetadataObserver_h
#define pybind_smtk_operation_MetadataObserver_h

#include <pybind11/pybind11.h>

#include "smtk/operation/MetadataObserver.h"

#include "smtk/operation/Metadata.h"

namespace py = pybind11;

py::class_< smtk::operation::MetadataObservers > pybind11_init_smtk_operation_MetadataObservers(py::module &m)
{
  py::class_< smtk::operation::MetadataObservers > instance(m, "MetadataObservers");
  instance
    .def(py::init<>())
    .def("__call__", [](smtk::operation::MetadataObservers& obs, const smtk::operation::Metadata& md, bool adding){ obs(md, adding); })
    .def("__len__", &smtk::operation::MetadataObservers::size)
    .def("insert", (smtk::operation::MetadataObservers::Key (smtk::operation::MetadataObservers::*)(smtk::operation::MetadataObserver, std::string)) &smtk::operation::MetadataObservers::insert, pybind11::keep_alive<1, 2>())
    .def("insert", (smtk::operation::MetadataObservers::Key (smtk::operation::MetadataObservers::*)(smtk::operation::MetadataObserver, smtk::operation::MetadataObservers::Priority, bool, std::string)) &smtk::operation::MetadataObservers::insert, pybind11::keep_alive<1, 2>())
    .def("erase", (std::size_t (smtk::operation::MetadataObservers::*)(smtk::operation::MetadataObservers::Key&)) &smtk::operation::MetadataObservers::erase)
    ;
  py::class_< smtk::operation::MetadataObservers::Key >(instance, "Key")
    .def(py::init<>())
    .def("assigned", &smtk::operation::MetadataObservers::Key::assigned)
    ;
  return instance;
}

#endif
