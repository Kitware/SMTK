//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Selection_h
#define pybind_smtk_mesh_Selection_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/resource/Selection.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::Selection, smtk::mesh::Component > pybind11_init_smtk_mesh_Selection(py::module &m)
{
  PySharedPtrClass< smtk::mesh::Selection, smtk::mesh::Component > instance(m, "Selection");
  instance
    .def(py::init<::smtk::mesh::Selection const &>())
    .def("deepcopy", (smtk::mesh::Selection & (smtk::mesh::Selection::*)(::smtk::mesh::Selection const &)) &smtk::mesh::Selection::operator=)
    .def("shared_from_this", (std::shared_ptr<smtk::mesh::Selection> (smtk::mesh::Selection::*)()) &smtk::mesh::Selection::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::mesh::Selection> (smtk::mesh::Selection::*)() const) &smtk::mesh::Selection::shared_from_this)
    .def_static("create", (std::shared_ptr<smtk::mesh::Selection> (*)(const smtk::mesh::CellSet&)) &smtk::mesh::Selection::create, py::arg("cellset"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Component> i) {
        return std::dynamic_pointer_cast<smtk::mesh::Selection>(i);
      })
    ;
  return instance;
}

#endif
