//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_FieldTypes_h
#define pybind_smtk_mesh_FieldTypes_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/core/FieldTypes.h"

namespace py = pybind11;

inline void pybind11_init_smtk_mesh_FieldType(py::module &m)
{
  py::enum_<smtk::mesh::FieldType>(m, "FieldType")
    .value("Integer", smtk::mesh::FieldType::Integer)
    .value("Double", smtk::mesh::FieldType::Double)
    .value("FieldType_MAX", smtk::mesh::FieldType::MaxFieldType)
    .export_values();
}

#endif
