//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_remus_MeshServerLauncher_h
#define pybind_smtk_extension_remus_MeshServerLauncher_h

#include <pybind11/pybind11.h>

#include "smtk/extension/remus/MeshServerLauncher.h"

namespace py = pybind11;

py::class_< smtk::mesh::MeshServerLauncher > pybind11_init_smtk_extension_remus_MeshServerLauncher(py::module &m)
{
  py::class_< smtk::mesh::MeshServerLauncher > instance(m, "MeshServerLauncher");
  instance
    .def(py::init<>())
    .def("launch", &smtk::mesh::MeshServerLauncher::launch)
    .def("isAlive", &smtk::mesh::MeshServerLauncher::isAlive)
    .def("terminate", &smtk::mesh::MeshServerLauncher::terminate)
    .def("addWorkerSearchDirectory", &smtk::mesh::MeshServerLauncher::addWorkerSearchDirectory)
    .def("clientEndpoint", &smtk::mesh::MeshServerLauncher::clientEndpoint)
    .def("clientHost", &smtk::mesh::MeshServerLauncher::clientHost)
    .def("clientScheme", &smtk::mesh::MeshServerLauncher::clientScheme)
    .def("clientPort", &smtk::mesh::MeshServerLauncher::clientPort)
    .def("workerEndpoint", &smtk::mesh::MeshServerLauncher::workerEndpoint)
    .def("workerHost", &smtk::mesh::MeshServerLauncher::workerHost)
    .def("workerScheme", &smtk::mesh::MeshServerLauncher::workerScheme)
    .def("workerPort", &smtk::mesh::MeshServerLauncher::workerPort)
    ;
  return instance;
}

#endif
