//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_SpatialData_h
#define pybind_smtk_markup_SpatialData_h

#include <pybind11/pybind11.h>

#include "smtk/markup/SpatialData.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::SpatialData> pybind11_init_smtk_markup_SpatialData(py::module &m)
{
  PySharedPtrClass< smtk::markup::SpatialData, smtk::markup::Component> instance(m, "SpatialData");
  instance
    .def("setBlanking", &smtk::markup::SpatialData::setBlanking, py::arg("shouldBlank"))
    .def("isBlanked", &smtk::markup::SpatialData::isBlanked)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::PersistentObject>& obj)
      { return std::dynamic_pointer_cast<smtk::markup::SpatialData>(obj); })
    ;
  return instance;
}

#endif
