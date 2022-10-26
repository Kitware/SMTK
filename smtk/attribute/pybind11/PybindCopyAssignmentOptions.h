//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_CopyAssignmentOptions_h
#define pybind_smtk_attribute_CopyAssignmentOptions_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/CopyAssignmentOptions.h"

namespace py = pybind11;

inline py::class_< smtk::attribute::AttributeCopyOptions > pybind11_init_smtk_attribute_AttributeCopyOptions(py::module &m)
{
  py::class_< smtk::attribute::AttributeCopyOptions > instance(m, "AttributeCopyOptions");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::AttributeCopyOptions const &>())
    .def("deepcopy", (smtk::attribute::AttributeCopyOptions & (smtk::attribute::AttributeCopyOptions::*)(::smtk::attribute::AttributeCopyOptions const &)) &smtk::attribute::AttributeCopyOptions::operator=)
    .def("copyUUID", &smtk::attribute::AttributeCopyOptions::copyUUID)
    .def("setCopyUUID", &smtk::attribute::AttributeCopyOptions::setCopyUUID, py::arg("val"))
    .def("copyDefinition", &smtk::attribute::AttributeCopyOptions::copyDefinition)
    .def("setCopyDefinition", &smtk::attribute::AttributeCopyOptions::setCopyDefinition, py::arg("val"))
    .def("convertToString", &smtk::attribute::AttributeCopyOptions::convertToString, py::arg("prefix") = "")
    ;
  return instance;
}

inline py::class_< smtk::attribute::AttributeAssignmentOptions > pybind11_init_smtk_attribute_AttributeAssignmentOptions(py::module &m)
{
  py::class_< smtk::attribute::AttributeAssignmentOptions > instance(m, "AttributeAssignmentOptions");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::AttributeAssignmentOptions const &>())
    .def("deepcopy", (smtk::attribute::AttributeAssignmentOptions & (smtk::attribute::AttributeAssignmentOptions::*)(::smtk::attribute::AttributeAssignmentOptions const &)) &smtk::attribute::AttributeAssignmentOptions::operator=)
    .def("ignoreMissingItems", &smtk::attribute::AttributeAssignmentOptions::ignoreMissingItems)
    .def("setIgnoreMissingItems", &smtk::attribute::AttributeAssignmentOptions::setIgnoreMissingItems, py::arg("val"))
    .def("copyAssociations", &smtk::attribute::AttributeAssignmentOptions::copyAssociations)
    .def("setCopyAssociations", &smtk::attribute::AttributeAssignmentOptions::setCopyAssociations, py::arg("val"))
    .def("allowPartialAssociations", &smtk::attribute::AttributeAssignmentOptions::allowPartialAssociations)
    .def("setAllowPartialAssociations", &smtk::attribute::AttributeAssignmentOptions::setAllowPartialAssociations, py::arg("val"))
    .def("doNotValidateAssociations", &smtk::attribute::AttributeAssignmentOptions::doNotValidateAssociations)
    .def("setDoNotValidateAssociations", &smtk::attribute::AttributeAssignmentOptions::setDoNotValidateAssociations, py::arg("val"))
    .def("convertToString", &smtk::attribute::AttributeAssignmentOptions::convertToString, py::arg("prefix") = "")
    ;
  return instance;
}

inline py::class_< smtk::attribute::ItemAssignmentOptions > pybind11_init_smtk_attribute_ItemAssignmentOptions(py::module &m)
{
  py::class_< smtk::attribute::ItemAssignmentOptions > instance(m, "ItemAssignmentOptions");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::ItemAssignmentOptions const &>())
    .def("deepcopy", (smtk::attribute::ItemAssignmentOptions & (smtk::attribute::ItemAssignmentOptions::*)(::smtk::attribute::ItemAssignmentOptions const &)) &smtk::attribute::ItemAssignmentOptions::operator=)
    .def("ignoreMissingChildren", &smtk::attribute::ItemAssignmentOptions::ignoreMissingChildren)
    .def("setIgnoreMissingChildren", &smtk::attribute::ItemAssignmentOptions::setIgnoreMissingChildren, py::arg("val"))
    .def("allowPartialValues", &smtk::attribute::ItemAssignmentOptions::allowPartialValues)
    .def("setAllowPartialValues", &smtk::attribute::ItemAssignmentOptions::setAllowPartialValues, py::arg("val"))
    .def("ignoreExpressions", &smtk::attribute::ItemAssignmentOptions::ignoreExpressions)
    .def("setIgnoreExpressions", &smtk::attribute::ItemAssignmentOptions::setIgnoreExpressions, py::arg("val"))
    .def("ignoreReferenceValues", &smtk::attribute::ItemAssignmentOptions::ignoreReferenceValues)
    .def("setIgnoreReferenceValues", &smtk::attribute::ItemAssignmentOptions::setIgnoreReferenceValues, py::arg("val"))
    .def("doNotValidateReferenceInfo", &smtk::attribute::ItemAssignmentOptions::doNotValidateReferenceInfo)
    .def("setDoNotValidateReferenceInfo", &smtk::attribute::ItemAssignmentOptions::setDoNotValidateReferenceInfo, py::arg("val"))
    .def("disableCopyAttributes", &smtk::attribute::ItemAssignmentOptions::disableCopyAttributes)
    .def("setDisableCopyAttributes", &smtk::attribute::ItemAssignmentOptions::setDisableCopyAttributes, py::arg("val"))
    .def("convertToString", &smtk::attribute::ItemAssignmentOptions::convertToString, py::arg("prefix") = "")
    ;
  return instance;
}

inline py::class_< smtk::attribute::CopyAssignmentOptions > pybind11_init_smtk_attribute_CopyAssignmentOptions(py::module &m)
{
  py::class_< smtk::attribute::CopyAssignmentOptions > instance(m, "CopyAssignmentOptions");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::CopyAssignmentOptions const &>())
    .def("deepcopy", (smtk::attribute::CopyAssignmentOptions & (smtk::attribute::CopyAssignmentOptions::*)(::smtk::attribute::CopyAssignmentOptions const &)) &smtk::attribute::CopyAssignmentOptions::operator=)
    .def("convertToString", &smtk::attribute::CopyAssignmentOptions::convertToString, py::arg("prefix") = "")
    .def_readwrite("copyOptions", &smtk::attribute::CopyAssignmentOptions::copyOptions)
    .def_readwrite("attributeOptions", &smtk::attribute::CopyAssignmentOptions::attributeOptions)
    .def_readwrite("itemOptions", &smtk::attribute::CopyAssignmentOptions::itemOptions)
    ;
  return instance;
}

#endif
