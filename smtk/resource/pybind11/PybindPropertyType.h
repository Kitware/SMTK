//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_PropertyType_h
#define pybind_smtk_resource_PropertyType_h

#include <pybind11/pybind11.h>

#include "smtk/resource/PropertyType.h"

namespace py = pybind11;

void pybind11_init_smtk_resource_PropertyType(py::module &m)
{
  py::enum_<smtk::resource::PropertyType>(m, "PropertyType")
    .value("FLOAT_PROPERTY", smtk::resource::PropertyType::FLOAT_PROPERTY)
    .value("STRING_PROPERTY", smtk::resource::PropertyType::STRING_PROPERTY)
    .value("INTEGER_PROPERTY", smtk::resource::PropertyType::INTEGER_PROPERTY)
    .value("INVALID_PROPERTY", smtk::resource::PropertyType::INVALID_PROPERTY)
    .export_values();
}

#endif
