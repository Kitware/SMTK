//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_UnstructuredData_h
#define pybind_smtk_markup_UnstructuredData_h

#include <pybind11/pybind11.h>

#include "smtk/markup/UnstructuredData.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::UnstructuredData> pybind11_init_smtk_markup_UnstructuredData(py::module &m)
{
  PySharedPtrClass< smtk::markup::UnstructuredData, smtk::markup::SpatialData> instance(m, "UnstructuredData");
  instance
    .def("shape", &smtk::markup::UnstructuredData::shape)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::PersistentObject>& obj)
      { return std::dynamic_pointer_cast<smtk::markup::UnstructuredData>(obj); })
    ;
  return instance;
}

#endif
