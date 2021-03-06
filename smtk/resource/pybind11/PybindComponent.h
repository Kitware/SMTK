//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Component_h
#define pybind_smtk_resource_Component_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Component.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

#include "smtk/resource/pybind11/PyComponent.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::resource::Component, smtk::resource::PyComponent, smtk::resource::PersistentObject > pybind11_init_smtk_resource_Component(py::module &m)
{
  PySharedPtrClass< smtk::resource::Component, smtk::resource::PyComponent, smtk::resource::PersistentObject > instance(m, "Component");
  instance
    .def(py::init<>())
    .def("deepcopy", (smtk::resource::Component & (smtk::resource::Component::*)(::smtk::resource::Component const &)) &smtk::resource::Component::operator=)
    .def("resource", &smtk::resource::Component::resource)
    ;
  return instance;
}

#endif
