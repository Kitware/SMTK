//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Component_h
#define pybind_smtk_common_Component_h

#include <pybind11/pybind11.h>

#include "smtk/common/Component.h"

#include "smtk/common/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

PySharedPtrClass< smtk::common::Component > pybind11_init_smtk_common_ResourceComponent(py::module &m)
{
  PySharedPtrClass< smtk::common::Component > instance(m, "ResourceComponent");
  instance
    .def("deepcopy", (smtk::common::Component & (smtk::common::ResourceComponent::*)(::smtk::common::ResourceComponent const &)) &smtk::common::ResourceComponent::operator=)
    .def("classname", &smtk::common::Component::classname)
    .def("id", &smtk::common::Component::id)
    .def("resource", &smtk::common::Component::resource)
    ;
  return instance;
}

#endif
