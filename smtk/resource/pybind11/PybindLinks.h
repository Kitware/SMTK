//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_Links_h
#define pybind_smtk_resource_Links_h

#include <pybind11/pybind11.h>

#include "smtk/resource/Links.h"
#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"

namespace py = pybind11;

inline py::class_< smtk::resource::Links> pybind11_init_smtk_resource_Links(py::module &m)
{
  py::class_< smtk::resource::Links> instance(m, "Links");
  instance
    .def_static("invalidRole", &smtk::resource::Links::invalidRole)
    .def_static("topLevelRole", &smtk::resource::Links::topLevelRole)
    .def("isLinkedTo", (bool (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Resource>&,  const smtk::resource::Links::RoleType&) const) &smtk::resource::Links::isLinkedTo)
    .def("isLinkedTo", (bool (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Component>&, const smtk::resource::Links::RoleType&) const) &smtk::resource::Links::isLinkedTo)
    .def("addLinkTo",  (smtk::resource::Links::Key (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Resource>&,  const smtk::resource::Links::RoleType&)) &smtk::resource::Links::addLinkTo)
    .def("addLinkTo",  (smtk::resource::Links::Key (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Component>&, const smtk::resource::Links::RoleType&)) &smtk::resource::Links::addLinkTo)
    .def("linkedTo", (smtk::resource::PersistentObjectSet (smtk::resource::Links::*)(const smtk::resource::Links::RoleType&) const) &smtk::resource::Links::linkedTo, py::arg("role"))
    .def("linkedFrom", (smtk::resource::PersistentObjectSet (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Resource>&, const smtk::resource::Links::RoleType&) const) &smtk::resource::Links::linkedFrom, py::arg("resource"), py::arg("role"))
    .def("removeLink", [](smtk::resource::Links& self, const smtk::common::UUID& rr, const smtk::common::UUID& rc)
      {
        smtk::resource::Links::Key key(rr, rc);
        return self.removeLink(key);
      }, py::arg("rhs_resource"), py::arg("rhs_component"))
    .def("removeLinksTo", (bool (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Resource>&,  const smtk::resource::Links::RoleType&)) &smtk::resource::Links::removeLinksTo)
    .def("removeLinksTo", (bool (smtk::resource::Links::*)(const std::shared_ptr<smtk::resource::Component>&, const smtk::resource::Links::RoleType&)) &smtk::resource::Links::removeLinksTo)

    .def("linkedObjectAndRole", [](const smtk::resource::Links& self, const smtk::common::UUID& rr, const smtk::common::UUID& rc)
      {
        smtk::resource::Links::Key key(rr, rc);
        return self.linkedObjectAndRole(key);
      }, py::arg("rhs_resource"), py::arg("rhs_component"))
    .def("linkedObject", [](const smtk::resource::Links& self, const smtk::common::UUID& rr, const smtk::common::UUID& rc)
      {
        smtk::resource::Links::Key key(rr, rc);
        return self.linkedObject(key);
      }, py::arg("rhs_resource"), py::arg("rhs_component"))

    .def("linkedObjectIdAndRole", [](const smtk::resource::Links& self, const smtk::common::UUID& rr, const smtk::common::UUID& rc)
      {
        smtk::resource::Links::Key key(rr, rc);
        return self.linkedObjectIdAndRole(key);
      }, py::arg("rhs_resource"), py::arg("rhs_component"))
    .def("linkedObjectId", [](const smtk::resource::Links& self, const smtk::common::UUID& rr, const smtk::common::UUID& rc)
      {
        smtk::resource::Links::Key key(rr, rc);
        return self.linkedObjectId(key);
      }, py::arg("rhs_resource"), py::arg("rhs_component"))
    ;
  return instance;
}

#endif
