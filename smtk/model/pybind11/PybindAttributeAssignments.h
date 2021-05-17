//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_AttributeAssignments_h
#define pybind_smtk_model_AttributeAssignments_h

#include <pybind11/pybind11.h>

#include "smtk/model/AttributeAssignments.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline py::class_< smtk::model::AttributeAssignments > pybind11_init_smtk_model_AttributeAssignments(py::module &m)
{
  py::class_< smtk::model::AttributeAssignments > instance(m, "AttributeAssignments");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::model::AttributeAssignments const &>())
    .def("deepcopy", (smtk::model::AttributeAssignments & (smtk::model::AttributeAssignments::*)(::smtk::model::AttributeAssignments const &)) &smtk::model::AttributeAssignments::operator=)
    .def("associateAttribute", &smtk::model::AttributeAssignments::associateAttribute, py::arg("attribId"))
    .def("disassociateAttribute", &smtk::model::AttributeAssignments::disassociateAttribute, py::arg("attribId"))
    .def("isAssociated", &smtk::model::AttributeAssignments::isAssociated, py::arg("attribId"))
    .def("attributeIds", (smtk::common::UUIDs & (smtk::model::AttributeAssignments::*)()) &smtk::model::AttributeAssignments::attributeIds)
    .def("attributeIds", (smtk::common::UUIDs const & (smtk::model::AttributeAssignments::*)() const) &smtk::model::AttributeAssignments::attributeIds)
    ;
  return instance;
}

#endif
