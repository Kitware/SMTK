//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_PersistentObject_h
#define pybind_smtk_resource_PersistentObject_h

#include <pybind11/pybind11.h>

#include "smtk/resource/PersistentObject.h"

namespace py = pybind11;

PySharedPtrClass< smtk::resource::PersistentObject > pybind11_init_smtk_resource_PersistentObject(py::module &m)
{
  PySharedPtrClass< smtk::resource::PersistentObject > instance(m, "PersistentObject");
  instance
    .def("deepcopy", (smtk::resource::PersistentObject & (smtk::resource::PersistentObject::*)(::smtk::resource::PersistentObject const &)) &smtk::resource::PersistentObject::operator=)
    .def("typeName", &smtk::resource::PersistentObject::typeName)
    .def("id", &smtk::resource::PersistentObject::id)
    .def("setId", &smtk::resource::PersistentObject::setId, py::arg("myID"))
    .def("name", &smtk::resource::PersistentObject::name)
    ;
  return instance;
}

#endif
