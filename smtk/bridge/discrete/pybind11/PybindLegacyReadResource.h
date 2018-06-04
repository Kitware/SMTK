//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_LegacyReadResource_h
#define pybind_smtk_bridge_discrete_operators_LegacyReadResource_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/LegacyReadResource.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::LegacyReadResource, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_LegacyReadResource(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::LegacyReadResource, smtk::operation::Operation > instance(m, "LegacyReadResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::LegacyReadResource> (*)()) &smtk::bridge::discrete::LegacyReadResource::create)
    .def("ableToOperate", &smtk::bridge::discrete::LegacyReadResource::ableToOperate)
    ;
  return instance;
}

#endif
