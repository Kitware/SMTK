//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_simulation_ExportSpec_h
#define pybind_smtk_simulation_ExportSpec_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/System.h"
#include "smtk/io/Logger.h"
#include "smtk/model/GridInfo.h"
#include "smtk/simulation/ExportSpec.h"

#include <sstream>
#include <string>

namespace py = pybind11;

py::class_< smtk::simulation::ExportSpec > pybind11_init_smtk_simulation_ExportSpec(py::module &m)
{
  py::class_< smtk::simulation::ExportSpec > instance(m, "ExportSpec");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::simulation::ExportSpec const &>())
    .def("deepcopy", (smtk::simulation::ExportSpec & (smtk::simulation::ExportSpec::*)(::smtk::simulation::ExportSpec const &)) &smtk::simulation::ExportSpec::operator=)
    .def("getSimulationAttributes", &smtk::simulation::ExportSpec::getSimulationAttributes)
    .def("getExportAttributes", &smtk::simulation::ExportSpec::getExportAttributes)
    .def("getAnalysisGridInfo", &smtk::simulation::ExportSpec::getAnalysisGridInfo)
    .def("getLogger", &smtk::simulation::ExportSpec::getLogger)
    .def("clear", &smtk::simulation::ExportSpec::clear)
    .def("setSimulationAttributes", &smtk::simulation::ExportSpec::setSimulationAttributes, py::arg("system"))
    .def("setExportAttributes", &smtk::simulation::ExportSpec::setExportAttributes, py::arg("system"))
    .def("setAnalysisGridInfo", &smtk::simulation::ExportSpec::setAnalysisGridInfo, py::arg("analysisGridInfo"))

    // Converter method used by CMB vtkPythonExporter
    .def_static("_InternalConverterDoNotUse_", [](const std::string& specAddressString) {
      unsigned long long memAddress;
      std::stringstream ss;
      ss << std::hex << specAddressString;
      ss >> memAddress;
      return reinterpret_cast<smtk::simulation::ExportSpec*>(memAddress);
      })

    ;
  return instance;
}

#endif
