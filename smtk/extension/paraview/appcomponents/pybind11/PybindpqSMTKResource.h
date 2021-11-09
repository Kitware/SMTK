//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKResource_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKResource_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"

namespace py = pybind11;

inline py::class_< pqSMTKResource, pqPipelineSource > pybind11_init_pqSMTKResource(py::module &m)
{
  py::class_< pqSMTKResource, pqPipelineSource > instance(m, "pqSMTKResource");
  instance
    .def(py::init<::QString const &, ::QString const &, ::vtkSMProxy *, ::pqServer *, ::QObject *>())
    .def("dropResource", &pqSMTKResource::dropResource)
    .def("getResource", &pqSMTKResource::getResource)
    .def("operationOccurred", &pqSMTKResource::operationOccurred, py::arg("arg0"))
    .def("resourceModified", &pqSMTKResource::resourceModified, py::arg("arg0"))
    ;
  return instance;
}

#endif
