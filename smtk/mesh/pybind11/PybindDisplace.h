//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_mesh_Displace_h
#define pybind_smtk_mesh_Displace_h

#include <pybind11/pybind11.h>

#include "smtk/mesh/Displace.h"

namespace py = pybind11;

PySharedPtrClass< smtk::mesh::ElevationControls > pybind11_init_smtk_mesh_ElevationControls(py::module &m)
{
  PySharedPtrClass< smtk::mesh::ElevationControls > instance(m, "ElevationControls");
  instance
    .def(py::init<::smtk::mesh::ElevationControls const &>())
    .def(py::init<>())
    .def(py::init<bool, double, bool, double, bool, double>())
    .def("deepcopy", (smtk::mesh::ElevationControls & (smtk::mesh::ElevationControls::*)(::smtk::mesh::ElevationControls const &)) &smtk::mesh::ElevationControls::operator=)
    .def_readwrite("m_clampMax", &smtk::mesh::ElevationControls::m_clampMax)
    .def_readwrite("m_clampMin", &smtk::mesh::ElevationControls::m_clampMin)
    .def_readwrite("m_invalid", &smtk::mesh::ElevationControls::m_invalid)
    .def_readwrite("m_maxElev", &smtk::mesh::ElevationControls::m_maxElev)
    .def_readwrite("m_minElev", &smtk::mesh::ElevationControls::m_minElev)
    .def_readwrite("m_useInvalid", &smtk::mesh::ElevationControls::m_useInvalid)
    ;
  return instance;
}

void pybind11_init__ZN4smtk4mesh8displaceERKNS0_8PointSetERKNS0_7MeshSetEd(py::module &m)
{
  m.def("displace", (bool (*)(::smtk::mesh::PointSet const &, ::smtk::mesh::MeshSet const &, double)) &smtk::mesh::displace, "", py::arg("pointcloud"), py::arg("ms"), py::arg("radius"));
}

void pybind11_init__ZN4smtk4mesh8displaceERKNS0_8PointSetES3_d(py::module &m)
{
  m.def("displace", (bool (*)(::smtk::mesh::PointSet const &, ::smtk::mesh::PointSet const &, double)) &smtk::mesh::displace, "", py::arg("pointcloud"), py::arg("arg1"), py::arg("radius"));
}

void pybind11_init__ZN4smtk4mesh7elevateERKNSt3__16vectorIdNS1_9allocatorIdEEEERKNS0_7MeshSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(::std::vector<double, std::allocator<double> > const &, ::smtk::mesh::MeshSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("ms"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

void pybind11_init__ZN4smtk4mesh7elevateERKNSt3__16vectorIdNS1_9allocatorIdEEEERKNS0_8PointSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(::std::vector<double, std::allocator<double> > const &, ::smtk::mesh::PointSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("ps"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

void pybind11_init__ZN4smtk4mesh7elevateEPKdmRKNS0_7MeshSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(double const * const, ::size_t, ::smtk::mesh::MeshSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("numPoints"), py::arg("ms"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

void pybind11_init__ZN4smtk4mesh7elevateEPKfmRKNS0_7MeshSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(float const * const, ::size_t, ::smtk::mesh::MeshSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("numPoints"), py::arg("ms"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

void pybind11_init__ZN4smtk4mesh7elevateEPKdmRKNS0_8PointSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(double const * const, ::size_t, ::smtk::mesh::PointSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("numPoints"), py::arg("ps"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

void pybind11_init__ZN4smtk4mesh7elevateEPKfmRKNS0_8PointSetEdNS0_17ElevationControlsE(py::module &m)
{
  m.def("elevate", (bool (*)(float const * const, ::size_t, ::smtk::mesh::PointSet const &, double, ::smtk::mesh::ElevationControls)) &smtk::mesh::elevate, "", py::arg("pointcloud"), py::arg("numPoints"), py::arg("ps"), py::arg("radius"), py::arg("controls") = smtk::mesh::ElevationControls());
}

#endif
