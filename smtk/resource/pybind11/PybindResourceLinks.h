//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_ResourceLinks_h
#define pybind_smtk_resource_ResourceLinks_h

#include <pybind11/pybind11.h>

#include "smtk/resource/ResourceLinks.h"

#include "smtk/resource/Links.h"

namespace py = pybind11;

inline py::class_< smtk::resource::detail::ResourceLinkBase > pybind11_init_smtk_resource_detail_ResourceLinkBase(py::module &m)
{
  py::class_< smtk::resource::detail::ResourceLinkBase > instance(m, "ResourceLinkBase");
  return instance;
}

inline py::class_< smtk::resource::detail::ResourceLinks, smtk::resource::Links > pybind11_init_smtk_resource_detail_ResourceLinks(py::module &m)
{
  py::class_< smtk::resource::detail::ResourceLinks, smtk::resource::Links > instance(m, "ResourceLinks");
  return instance;
}

#endif
