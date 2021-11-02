//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKWrapper_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKWrapper_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

namespace py = pybind11;

inline py::class_< pqSMTKWrapper, pqProxy > pybind11_init_pqSMTKWrapper(py::module &m)
{
  py::class_< pqSMTKWrapper, pqProxy > instance(m, "pqSMTKWrapper");
  instance
    .def(py::init<::QString const &, ::QString const &, ::vtkSMProxy *, ::pqServer *, ::QObject *>())
    .def("addResource", &pqSMTKWrapper::addResource, py::arg("rsrc"))
    .def("getPVResource", &pqSMTKWrapper::getPVResource, py::arg("rsrc"))
    .def("removeResource", &pqSMTKWrapper::removeResource, py::arg("rsrc"))
    .def("smtkManagers", &pqSMTKWrapper::smtkManagers)
    .def("smtkOperationManager", &pqSMTKWrapper::smtkOperationManager)
    .def("smtkProjectManager", &pqSMTKWrapper::smtkProjectManager)
    .def("smtkProxy", &pqSMTKWrapper::smtkProxy)
    .def("smtkResourceManager", &pqSMTKWrapper::smtkResourceManager)
    .def("smtkSelection", &pqSMTKWrapper::smtkSelection)
    .def("smtkViewManager", &pqSMTKWrapper::smtkViewManager)
    .def("visitResources", &pqSMTKWrapper::visitResources, py::arg("visitor"))
    ;
  return instance;
}

#endif
