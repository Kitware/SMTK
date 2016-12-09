//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_DimensionTypes_h
#define pybind_smtk_mesh_DimensionTypes_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/DimensionTypes.h"

namespace py = pybind11;

void pybind11_init_smtk_mesh_DimensionType(py::module &m)
{
  py::enum_<smtk::mesh::DimensionType>(m, "DimensionType")
    .value("Dims0", smtk::mesh::DimensionType::Dims0)
    .value("Dims1", smtk::mesh::DimensionType::Dims1)
    .value("Dims2", smtk::mesh::DimensionType::Dims2)
    .value("Dims3", smtk::mesh::DimensionType::Dims3)
    .value("DimensionType_MAX", smtk::mesh::DimensionType::DimensionType_MAX)
    .export_values();
}

#endif
