//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Queries_h
#define pybind_smtk_attribute_Queries_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/utility/Queries.h"

namespace py = pybind11;


inline void pybind11_init_smtk_attribute_queries(py::module &m)
{
  py::enum_<smtk::attribute::utility::EditableParameters>(m, "EditableParameters")
    .value("None", smtk::attribute::utility::EditableParameters::None)
    .value("Optional", smtk::attribute::utility::EditableParameters::Optional)
    .value("Mandatory", smtk::attribute::utility::EditableParameters::Mandatory)
    .export_values();

  m.def("checkUniquenessCondition", &smtk::attribute::utility::checkUniquenessCondition);
  m.def("associatableObjects", (std::set<smtk::resource::PersistentObjectPtr> (*)(const smtk::attribute::ConstReferenceItemDefinitionPtr&,
    smtk::attribute::ResourcePtr&, smtk::resource::ManagerPtr&, const smtk::common::UUID&)) &smtk::attribute::utility::associatableObjects, py::arg("refItemDef"),
    py::arg("attResource"), py::arg("resManager"), py::arg("ignoreResource"));
  m.def("associatableObjects", (std::set<smtk::resource::PersistentObjectPtr> (*)(const smtk::attribute::ReferenceItemPtr&,
    smtk::resource::ManagerPtr&, bool useAttributeAssociations, const smtk::common::UUID&)) &smtk::attribute::utility::associatableObjects, py::arg("refItem"),
    py::arg("resManager"), py::arg("useAttributeAssociations"), py::arg("ignoreResourceUUID"));
  m.def("userEditableParameters", &smtk::attribute::utility::userEditableParameters);
}

#endif
