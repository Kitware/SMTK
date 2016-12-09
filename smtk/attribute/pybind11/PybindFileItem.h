//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_FileItem_h
#define pybind_smtk_attribute_FileItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/FileItem.h"

#include "smtk/attribute/FileSystemItem.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::FileItem, smtk::attribute::FileSystemItem > pybind11_init_smtk_attribute_FileItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::FileItem, smtk::attribute::FileSystemItem > instance(m, "FileItem");
  instance
    .def(py::init<::smtk::attribute::FileItem const &>())
    .def("deepcopy", (smtk::attribute::FileItem & (smtk::attribute::FileItem::*)(::smtk::attribute::FileItem const &)) &smtk::attribute::FileItem::operator=)
    .def("addRecentValue", &smtk::attribute::FileItem::addRecentValue, py::arg("val"))
    .def("classname", &smtk::attribute::FileItem::classname)
    .def("recentValues", &smtk::attribute::FileItem::recentValues)
    .def("type", &smtk::attribute::FileItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::FileItem>(i);
      })
    ;
  return instance;
}

#endif
