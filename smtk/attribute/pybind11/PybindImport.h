//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_operators_Import_h
#define pybind_smtk_attribute_operators_Import_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/operators/Import.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Import, smtk::operation::XMLOperation > pybind11_init_smtk_attribute_Import(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Import, smtk::operation::XMLOperation > instance(m, "Import");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::attribute::Import> (*)()) &smtk::attribute::Import::create)
    .def_static("create", (std::shared_ptr<smtk::attribute::Import> (*)(::std::shared_ptr<smtk::attribute::Import> &)) &smtk::attribute::Import::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::attribute::Import> (smtk::attribute::Import::*)() const) &smtk::attribute::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::attribute::Import> (smtk::attribute::Import::*)()) &smtk::attribute::Import::shared_from_this)
    ;
  return instance;
}

#endif
