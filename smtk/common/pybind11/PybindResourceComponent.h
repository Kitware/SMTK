//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_ResourceComponent_h
#define pybind_smtk_common_ResourceComponent_h

#include <pybind11/pybind11.h>

#include "smtk/common/ResourceComponent.h"

#include "smtk/common/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

PySharedPtrClass< smtk::common::ResourceComponent > pybind11_init_smtk_common_ResourceComponent(py::module &m)
{
  PySharedPtrClass< smtk::common::ResourceComponent > instance(m, "ResourceComponent");
  instance
    .def("deepcopy", (smtk::common::ResourceComponent & (smtk::common::ResourceComponent::*)(::smtk::common::ResourceComponent const &)) &smtk::common::ResourceComponent::operator=)
    .def("classname", &smtk::common::ResourceComponent::classname)
    .def("id", &smtk::common::ResourceComponent::id)
    .def("resource", &smtk::common::ResourceComponent::resource)
    ;
  return instance;
}

#endif
