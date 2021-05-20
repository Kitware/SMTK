//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Container_h
#define pybind_smtk_project_Container_h

#include <pybind11/pybind11.h>

#include "smtk/project/Container.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/project/Project.h"

namespace py = pybind11;

inline void pybind11_init_smtk_project_detail_id(py::module &m)
{
  m.def("id", &smtk::project::detail::id, "", py::arg("r"));
}

inline void pybind11_init_smtk_project_detail_index(py::module &m)
{
  m.def("index", &smtk::project::detail::index, "", py::arg("r"));
}

inline void pybind11_init_smtk_project_detail_location(py::module &m)
{
  m.def("location", &smtk::project::detail::location, "", py::arg("r"));
}

inline void pybind11_init_smtk_project_detail_name(py::module &m)
{
  m.def("name", &smtk::project::detail::name, "", py::arg("r"));
}

#endif
