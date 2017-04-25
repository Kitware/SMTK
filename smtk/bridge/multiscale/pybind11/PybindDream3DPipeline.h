//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_Dream3DPipeline_h
#define pybind_smtk_bridge_multiscale_operators_Dream3DPipeline_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/Dream3DPipeline.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipeline, smtk::bridge::multiscale::PythonScript > pybind11_init_smtk_bridge_multiscale_Dream3DPipeline(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipeline, smtk::bridge::multiscale::PythonScript > instance(m, "Dream3DPipeline");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::Dream3DPipeline const &>())
    .def("deepcopy", (smtk::bridge::multiscale::Dream3DPipeline & (smtk::bridge::multiscale::Dream3DPipeline::*)(::smtk::bridge::multiscale::Dream3DPipeline const &)) &smtk::bridge::multiscale::Dream3DPipeline::operator=)
    .def_static("baseCreate", &smtk::bridge::multiscale::Dream3DPipeline::baseCreate)
    .def("className", &smtk::bridge::multiscale::Dream3DPipeline::className)
    .def("classname", &smtk::bridge::multiscale::Dream3DPipeline::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipeline> (*)()) &smtk::bridge::multiscale::Dream3DPipeline::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipeline> (*)(::std::shared_ptr<smtk::bridge::multiscale::Dream3DPipeline> &)) &smtk::bridge::multiscale::Dream3DPipeline::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::Dream3DPipeline::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::Dream3DPipeline> (smtk::bridge::multiscale::Dream3DPipeline::*)() const) &smtk::bridge::multiscale::Dream3DPipeline::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipeline> (smtk::bridge::multiscale::Dream3DPipeline::*)()) &smtk::bridge::multiscale::Dream3DPipeline::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::bridge::multiscale::Dream3DPipeline::operatorName)
    ;
  return instance;
}

#endif
