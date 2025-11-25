//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_DiscreteGeometry_h
#define pybind_smtk_markup_DiscreteGeometry_h

#include <pybind11/pybind11.h>

#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/extension/vtk/pybind11/PybindVTKTypeCaster.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::DiscreteGeometry> pybind11_init_smtk_markup_DiscreteGeometry(py::module &m)
{
  PySharedPtrClass< smtk::markup::DiscreteGeometry, smtk::markup::SpatialData> instance(m, "DiscreteGeometry");
  instance
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::PersistentObject>& obj)
      { return std::dynamic_pointer_cast<smtk::markup::DiscreteGeometry>(obj); })
    ;
  py::class_<smtk::markup::DiscreteGeometry::ShapeOptions> shapeOpts(instance, "ShapeOptions");
  shapeOpts
    .def(py::init<>())
    .def(py::init<::smtk::markup::DiscreteGeometry::ShapeOptions const &>())
    .def("trackedChanges",
      [](smtk::markup::DiscreteGeometry::ShapeOptions& opts)
      { return opts.trackedChanges; })
    .def("setTrackedChanges",
      [](smtk::markup::DiscreteGeometry::ShapeOptions& opts, smtk::operation::Operation::Result res)
      { opts.trackedChanges = res; }, py::arg("tracked_changes"))
    ;
  return instance;
}

#endif
