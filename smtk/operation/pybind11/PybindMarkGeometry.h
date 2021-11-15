//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_MarkGeometry_h
#define pybind_smtk_operation_MarkGeometry_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/operation/MarkGeometry.h"

#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Operation.h"

#include "smtk/operation/pybind11/PyOperation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include <vector>

namespace py = pybind11;

inline py::class_< smtk::operation::MarkGeometry > pybind11_init_smtk_operation_MarkGeometry(py::module &m)
{
  py::class_< smtk::operation::MarkGeometry > instance(m, "MarkGeometry");
  instance
    .def(py::init<>())
    .def(py::init<std::shared_ptr<smtk::geometry::Resource>>())
    .def("markModified", (void (smtk::operation::MarkGeometry::*)(const smtk::resource::PersistentObjectPtr&)) &smtk::operation::MarkGeometry::markModified, py::arg("object"))
    .def("markModified", (void (smtk::operation::MarkGeometry::*)(const smtk::attribute::ReferenceItemPtr&)) &smtk::operation::MarkGeometry::markModified, py::arg("item"))
    .def("erase", (void (smtk::operation::MarkGeometry::*)(const smtk::resource::PersistentObjectPtr&)) &smtk::operation::MarkGeometry::erase, py::arg("object"))
    .def("erase", (void (smtk::operation::MarkGeometry::*)(const smtk::common::UUID&)) &smtk::operation::MarkGeometry::erase, py::arg("id"))
    .def("erase", (void (smtk::operation::MarkGeometry::*)(const smtk::attribute::ReferenceItemPtr&)) &smtk::operation::MarkGeometry::erase, py::arg("item"))
    .def("markResult", &smtk::operation::MarkGeometry::markResult, py::arg("result"))
    ;
  return instance;
}

#endif
