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

#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

PySharedPtrClass< smtk::resource::Component > pybind11_init_smtk_resource_Component(py::module &m)
{
  PySharedPtrClass< smtk::resource::Component > instance(m, "Component");
  instance
    .def("deepcopy", (smtk::resource::Component & (smtk::resource::Component::*)(::smtk::resource::Component const &)) &smtk::resource::Component::operator=)
    .def("classname", &smtk::resource::Component::classname)
    .def("id", &smtk::resource::Component::id)
    .def("resource", &smtk::resource::Component::resource)
    ;
  return instance;
}

#endif
