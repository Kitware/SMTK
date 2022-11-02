//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_FileItemDefinition_h
#define pybind_smtk_attribute_FileItemDefinition_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/FileItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::FileItemDefinition, smtk::attribute::FileSystemItemDefinition > pybind11_init_smtk_attribute_FileItemDefinition(py::module &m)
{
  PySharedPtrClass< smtk::attribute::FileItemDefinition, smtk::attribute::FileSystemItemDefinition > instance(m, "FileItemDefinition");
  instance
    .def(py::init<::smtk::attribute::FileItemDefinition const &>())
    .def_static("New", &smtk::attribute::FileItemDefinition::New, py::arg("myName"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::FileItemDefinition::*)(::smtk::attribute::Attribute *, int) const) &smtk::attribute::FileItemDefinition::buildItem, py::arg("owningAttribute"), py::arg("itemPosition"))
    .def("buildItem", (smtk::attribute::ItemPtr (smtk::attribute::FileItemDefinition::*)(::smtk::attribute::Item *, int, int) const) &smtk::attribute::FileItemDefinition::buildItem, py::arg("owningItem"), py::arg("position"), py::arg("subGroupPosition"))
    .def("createCopy", &smtk::attribute::FileItemDefinition::createCopy, py::arg("info"))
    .def("getFileFilters", &smtk::attribute::FileItemDefinition::getFileFilters)
    .def("setFileFilters", &smtk::attribute::FileItemDefinition::setFileFilters, py::arg("filters"))
    .def("getSummarizedFileFilters", &smtk::attribute::FileItemDefinition::getSummarizedFileFilters)
    .def_static("aggregateFileFilters", [](const std::string& filters)
      {
        int count;
        auto agg = smtk::attribute::FileItemDefinition::aggregateFileFilters(filters, count);
        return std::make_pair(agg, count);
      })
    .def("type", &smtk::attribute::FileItemDefinition::type)
    .def_static("ToItemDefinition", [](const std::shared_ptr<smtk::attribute::FileItemDefinition> d) {
        return std::dynamic_pointer_cast<smtk::attribute::ItemDefinition>(d);
      })
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::ItemDefinition> i) {
        return std::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(i);
      })
    ;
  return instance;
}

#endif
