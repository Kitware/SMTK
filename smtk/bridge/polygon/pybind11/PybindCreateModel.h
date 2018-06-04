//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_operators_CreateModel_h
#define pybind_smtk_bridge_polygon_operators_CreateModel_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/operators/CreateModel.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::polygon::CreateModel > pybind11_init_smtk_bridge_polygon_CreateModel(py::module &m, PySharedPtrClass< smtk::bridge::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::bridge::polygon::CreateModel > instance(m, "CreateModel", parent);
  instance
    .def(py::init<::smtk::bridge::polygon::CreateModel const &>())
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::CreateModel> (*)()) &smtk::bridge::polygon::CreateModel::create)
    ;
  return instance;
}

#endif
