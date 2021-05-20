//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Component_h
#define pybind_smtk_mesh_Component_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/Component.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/resource/Component.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::mesh::Component, smtk::resource::Component > pybind11_init_smtk_mesh_Component(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Component, smtk::resource::Component > instance(m, "Component");
  instance
    .def(py::init<::smtk::mesh::Component const &>())
    .def("deepcopy", (smtk::mesh::Component & (smtk::mesh::Component::*)(::smtk::mesh::Component const &)) &smtk::mesh::Component::operator=)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::Component> (smtk::mesh::Component::*)()) &smtk::mesh::Component::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::Component> (smtk::mesh::Component::*)() const) &smtk::mesh::Component::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::mesh::Component> (*)(const smtk::mesh::ResourcePtr&, const smtk::common::UUID&)) &smtk::mesh::Component::create, py::arg("resource"), py::arg("id"))
    .def_static("create", (std::shared_ptr<smtk::mesh::Component> (*)(const smtk::mesh::MeshSet&)) &smtk::mesh::Component::create, py::arg("meshset"))
    .def("resource", &smtk::mesh::Component::resource)
    .def("mesh", (smtk::mesh::MeshSet (smtk::mesh::Component::*)()) &smtk::mesh::Component::mesh)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Component> i) {
        return std::dynamic_pointer_cast<smtk::mesh::Component>(i);
      })
    ;
  return instance;
}

#endif
