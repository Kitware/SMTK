//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_CopyOptions_h
#define pybind_smtk_resource_CopyOptions_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/resource/CopyOptions.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline py::class_< smtk::resource::CopyOptions > pybind11_init_smtk_resource_CopyOptions(py::module &m)
{
  py::class_< smtk::resource::CopyOptions > instance(m, "CopyOptions");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::io::Logger&>())
    .def("copyLocation", &smtk::resource::CopyOptions::copyLocation)
    .def("setCopyLocation", &smtk::resource::CopyOptions::setCopyLocation)
    .def("copyComponents", &smtk::resource::CopyOptions::copyComponents)
    .def("setCopyComponents", &smtk::resource::CopyOptions::setCopyComponents)
    .def("copyProperties", &smtk::resource::CopyOptions::copyProperties)
    .def("setCopyProperties", &smtk::resource::CopyOptions::setCopyProperties)
    .def("copyGeometry", &smtk::resource::CopyOptions::copyGeometry)
    .def("setCopyGeometry", &smtk::resource::CopyOptions::setCopyGeometry)
    .def("copyTemplateData", &smtk::resource::CopyOptions::copyTemplateData)
    .def("setCopyTemplateData", &smtk::resource::CopyOptions::setCopyTemplateData)
    .def("copyTemplateVersion", &smtk::resource::CopyOptions::copyTemplateVersion)
    .def("setCopyTemplateVersion", &smtk::resource::CopyOptions::setCopyTemplateVersion)
    .def("copyUnitSystem", &smtk::resource::CopyOptions::copyUnitSystem)
    .def("setCopyUnitSystem", &smtk::resource::CopyOptions::setCopyUnitSystem)
    .def("copyLinks", &smtk::resource::CopyOptions::copyLinks)
    .def("setCopyLinks", &smtk::resource::CopyOptions::setCopyLinks)
    .def("clearLinkRolesToExclude", &smtk::resource::CopyOptions::clearLinkRolesToExclude)
    .def("addLinkRoleToExclude", &smtk::resource::CopyOptions::addLinkRoleToExclude)
    .def("removeLinkRoleToExclude", &smtk::resource::CopyOptions::removeLinkRoleToExclude)
    .def("linkRolesToExclude", &smtk::resource::CopyOptions::linkRolesToExclude)
    .def("shouldExcludeLinksInRole", &smtk::resource::CopyOptions::shouldExcludeLinksInRole)
    .def("omit", (std::unordered_set<smtk::common::UUID>& (smtk::resource::CopyOptions::*)())&smtk::resource::CopyOptions::omit)
    .def("shouldOmitId", &smtk::resource::CopyOptions::shouldOmitId)
    .def("omitComponents", &smtk::resource::CopyOptions::omitComponents, py::arg("resource"))
    .def("objectMapping", (smtk::resource::CopyOptions::ObjectMapType& (smtk::resource::CopyOptions::*)())&smtk::resource::CopyOptions::objectMapping)
    .def("log", &smtk::resource::CopyOptions::log)
    ;
  return instance;
}

#endif
