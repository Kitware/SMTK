//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_Dream3DPipelineOperator_h
#define pybind_smtk_bridge_multiscale_operators_Dream3DPipelineOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/Dream3DPipelineOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipelineOperator, smtk::bridge::multiscale::PythonScriptOperator > pybind11_init_smtk_bridge_multiscale_Dream3DPipelineOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::Dream3DPipelineOperator, smtk::bridge::multiscale::PythonScriptOperator > instance(m, "Dream3DPipelineOperator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::Dream3DPipelineOperator const &>())
    .def("deepcopy", (smtk::bridge::multiscale::Dream3DPipelineOperator & (smtk::bridge::multiscale::Dream3DPipelineOperator::*)(::smtk::bridge::multiscale::Dream3DPipelineOperator const &)) &smtk::bridge::multiscale::Dream3DPipelineOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::multiscale::Dream3DPipelineOperator::baseCreate)
    .def("className", &smtk::bridge::multiscale::Dream3DPipelineOperator::className)
    .def("classname", &smtk::bridge::multiscale::Dream3DPipelineOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipelineOperator> (*)()) &smtk::bridge::multiscale::Dream3DPipelineOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipelineOperator> (*)(::std::shared_ptr<smtk::bridge::multiscale::Dream3DPipelineOperator> &)) &smtk::bridge::multiscale::Dream3DPipelineOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::Dream3DPipelineOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::Dream3DPipelineOperator> (smtk::bridge::multiscale::Dream3DPipelineOperator::*)() const) &smtk::bridge::multiscale::Dream3DPipelineOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::Dream3DPipelineOperator> (smtk::bridge::multiscale::Dream3DPipelineOperator::*)()) &smtk::bridge::multiscale::Dream3DPipelineOperator::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::bridge::multiscale::Dream3DPipelineOperator::operatorName)
    ;
  return instance;
}

#endif
