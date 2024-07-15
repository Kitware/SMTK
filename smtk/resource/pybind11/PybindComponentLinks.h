//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_ComponentLinks_h
#define pybind_smtk_resource_ComponentLinks_h

#include <pybind11/pybind11.h>

#include "smtk/resource/ComponentLinks.h"

#include "smtk/resource/Links.h"

namespace py = pybind11;

inline py::class_< smtk::resource::detail::ComponentLinkBase > pybind11_init_smtk_resource_detail_ComponentLinkBase(py::module &m)
{
  py::class_< smtk::resource::detail::ComponentLinkBase > instance(m, "ComponentLinkBase");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::resource::detail::ComponentLinkBase const &>())
    ;
  return instance;
}

inline py::class_< smtk::resource::detail::ComponentLinks, smtk::resource::Links > pybind11_init_smtk_resource_detail_ComponentLinks(py::module &m)
{
  py::class_< smtk::resource::detail::ComponentLinks, smtk::resource::Links > instance(m, "ComponentLinks");
  instance
    .def(py::init<::smtk::resource::detail::ComponentLinks const &>())
    ;
  return instance;
}

#endif
