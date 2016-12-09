//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_DirectoryItem_h
#define pybind_smtk_attribute_DirectoryItem_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/DirectoryItem.h"

#include "smtk/attribute/FileSystemItem.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::DirectoryItem, smtk::attribute::FileSystemItem > pybind11_init_smtk_attribute_DirectoryItem(py::module &m)
{
  PySharedPtrClass< smtk::attribute::DirectoryItem, smtk::attribute::FileSystemItem > instance(m, "DirectoryItem");
  instance
    .def(py::init<::smtk::attribute::DirectoryItem const &>())
    .def("deepcopy", (smtk::attribute::DirectoryItem & (smtk::attribute::DirectoryItem::*)(::smtk::attribute::DirectoryItem const &)) &smtk::attribute::DirectoryItem::operator=)
    .def("classname", &smtk::attribute::DirectoryItem::classname)
    .def("type", &smtk::attribute::DirectoryItem::type)
    .def_static("CastTo", [](const std::shared_ptr<smtk::attribute::Item> i) {
        return std::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(i);
      })
    ;
  return instance;
}

#endif
