//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Metrics_h
#define pybind_smtk_mesh_Metrics_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/mesh/utility/Metrics.h"

namespace py = pybind11;


inline void pybind11_init_smtk_mesh_metrics(py::module &m)
{
  m.def("extent", &smtk::mesh::utility::extent);
  m.def("highestDimension", &smtk::mesh::utility::highestDimension);
  m.def("eulerCharacteristic", &smtk::mesh::utility::eulerCharacteristic);
}

#endif
