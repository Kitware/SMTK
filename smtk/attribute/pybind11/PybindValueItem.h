//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_ValueItem_h
#define pybind_smtk_attribute_ValueItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ValueItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/SearchStyle.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::ValueItem, smtk::attribute::Item > pybind11_init_smtk_attribute_ValueItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::ValueItem, smtk::attribute::Item > instance(m, "ValueItem");
  instance
    .def("deepcopy", (smtk::attribute::ValueItem & (smtk::attribute::ValueItem::*)(::smtk::attribute::ValueItem const &)) &smtk::attribute::ValueItem::operator=)
    .def("activeChildItem", &smtk::attribute::ValueItem::activeChildItem, py::arg("i"))
    .def("allowsExpressions", &smtk::attribute::ValueItem::allowsExpressions)
    .def("appendExpression", &smtk::attribute::ValueItem::appendExpression, py::arg("exp"))
    .def("assign", &smtk::attribute::ValueItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("childrenItems", &smtk::attribute::ValueItem::childrenItems)
    .def("classname", &smtk::attribute::ValueItem::classname)
    .def("discreteIndex", &smtk::attribute::ValueItem::discreteIndex, py::arg("elementIndex") = 0)
    .def("expression", &smtk::attribute::ValueItem::expression, py::arg("elementIndex") = 0)
    .def("expressionReference", &smtk::attribute::ValueItem::expressionReference, py::arg("elementIndex") = 0)
    .def("findChild", (smtk::attribute::ItemPtr (smtk::attribute::ValueItem::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::ValueItem::findChild, py::arg("name"), py::arg("arg1"))
    .def("findChild", (smtk::attribute::ConstItemPtr (smtk::attribute::ValueItem::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::ValueItem::findChild, py::arg("name"), py::arg("arg1"))
    .def("hasDefault", &smtk::attribute::ValueItem::hasDefault)
    .def("isDiscrete", &smtk::attribute::ValueItem::isDiscrete)
    .def("isDiscreteIndexValid", &smtk::attribute::ValueItem::isDiscreteIndexValid, py::arg("value"))
    .def("isExpression", &smtk::attribute::ValueItem::isExpression, py::arg("elementIndex") = 0)
    .def("isExtensible", &smtk::attribute::ValueItem::isExtensible)
    .def("isSet", &smtk::attribute::ValueItem::isSet, py::arg("elementIndex") = 0)
    .def("isUsingDefault", (bool (smtk::attribute::ValueItem::*)(::size_t) const) &smtk::attribute::ValueItem::isUsingDefault, py::arg("elementIndex"))
    .def("isUsingDefault", (bool (smtk::attribute::ValueItem::*)() const) &smtk::attribute::ValueItem::isUsingDefault)
    .def("isValid", &smtk::attribute::ValueItem::isValid)
    .def("maxNumberOfValues", &smtk::attribute::ValueItem::maxNumberOfValues)
    .def("numberOfActiveChildrenItems", &smtk::attribute::ValueItem::numberOfActiveChildrenItems)
    .def("numberOfChildrenItems", &smtk::attribute::ValueItem::numberOfChildrenItems)
    .def("numberOfRequiredValues", &smtk::attribute::ValueItem::numberOfRequiredValues)
    .def("numberOfValues", &smtk::attribute::ValueItem::numberOfValues)
    .def("reset", &smtk::attribute::ValueItem::reset)
    .def("setDiscreteIndex", (bool (smtk::attribute::ValueItem::*)(int)) &smtk::attribute::ValueItem::setDiscreteIndex, py::arg("value"))
    .def("setDiscreteIndex", (bool (smtk::attribute::ValueItem::*)(::size_t, int)) &smtk::attribute::ValueItem::setDiscreteIndex, py::arg("elementIndex"), py::arg("value"))
    .def("setExpression", (bool (smtk::attribute::ValueItem::*)(::smtk::attribute::AttributePtr)) &smtk::attribute::ValueItem::setExpression, py::arg("exp"))
    .def("setExpression", (bool (smtk::attribute::ValueItem::*)(::size_t, ::smtk::attribute::AttributePtr)) &smtk::attribute::ValueItem::setExpression, py::arg("elementIndex"), py::arg("exp"))
    .def("setNumberOfValues", &smtk::attribute::ValueItem::setNumberOfValues, py::arg("newSize"))
    .def("setToDefault", &smtk::attribute::ValueItem::setToDefault, py::arg("elementIndex") = 0)
    .def("unset", &smtk::attribute::ValueItem::unset, py::arg("elementIndex") = 0)
    .def("valueAsString", (std::string (smtk::attribute::ValueItem::*)() const) &smtk::attribute::ValueItem::valueAsString)
    .def("valueAsString", (std::string (smtk::attribute::ValueItem::*)(::size_t) const) &smtk::attribute::ValueItem::valueAsString, py::arg("elementIndex"))
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::ValueItem>(i);
      })
    ;
  return instance;
}

#endif
