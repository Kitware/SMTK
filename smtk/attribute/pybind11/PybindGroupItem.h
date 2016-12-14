//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_GroupItem_h
#define pybind_smtk_attribute_GroupItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/GroupItem.h"

#include "smtk/attribute/Item.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::GroupItem, smtk::attribute::Item > pybind11_init_smtk_attribute_GroupItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::GroupItem, smtk::attribute::Item > instance(m, "GroupItem");
  instance
    .def(py::init<::smtk::attribute::GroupItem const &>())
    .def("deepcopy", (smtk::attribute::GroupItem & (smtk::attribute::GroupItem::*)(::smtk::attribute::GroupItem const &)) &smtk::attribute::GroupItem::operator=)
    .def("appendGroup", &smtk::attribute::GroupItem::appendGroup)
    .def("assign", &smtk::attribute::GroupItem::assign, py::arg("sourceItem"), py::arg("options") = 0)
    .def("begin", &smtk::attribute::GroupItem::begin)
    .def("classname", &smtk::attribute::GroupItem::classname)
    .def("end", &smtk::attribute::GroupItem::end)
    .def("find", (smtk::attribute::ItemPtr (smtk::attribute::GroupItem::*)(::std::string const &)) &smtk::attribute::GroupItem::find, py::arg("inName"))
    .def("find", (smtk::attribute::ItemPtr (smtk::attribute::GroupItem::*)(::size_t, ::std::string const &)) &smtk::attribute::GroupItem::find, py::arg("element"), py::arg("name"))
    .def("find", (smtk::attribute::ConstItemPtr (smtk::attribute::GroupItem::*)(::std::string const &) const) &smtk::attribute::GroupItem::find, py::arg("inName"))
    .def("find", (smtk::attribute::ConstItemPtr (smtk::attribute::GroupItem::*)(::size_t, ::std::string const &) const) &smtk::attribute::GroupItem::find, py::arg("element"), py::arg("name"))
    .def("isExtensible", &smtk::attribute::GroupItem::isExtensible)
    .def("isValid", &smtk::attribute::GroupItem::isValid)
    .def("item", (smtk::attribute::ItemPtr (smtk::attribute::GroupItem::*)(::size_t) const) &smtk::attribute::GroupItem::item, py::arg("ith"))
    .def("item", (smtk::attribute::ItemPtr (smtk::attribute::GroupItem::*)(::size_t, ::size_t) const) &smtk::attribute::GroupItem::item, py::arg("element"), py::arg("ith"))
    .def("maxNumberOfGroups", &smtk::attribute::GroupItem::maxNumberOfGroups)
    .def("numberOfGroups", &smtk::attribute::GroupItem::numberOfGroups)
    .def("numberOfItemsPerGroup", &smtk::attribute::GroupItem::numberOfItemsPerGroup)
    .def("numberOfRequiredGroups", &smtk::attribute::GroupItem::numberOfRequiredGroups)
    .def("removeGroup", &smtk::attribute::GroupItem::removeGroup, py::arg("element"))
    .def("reset", &smtk::attribute::GroupItem::reset)
    .def("setNumberOfGroups", &smtk::attribute::GroupItem::setNumberOfGroups, py::arg("newSize"))
    .def("type", &smtk::attribute::GroupItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::GroupItem>(i);
      })
    ;
  return instance;
}

#endif
