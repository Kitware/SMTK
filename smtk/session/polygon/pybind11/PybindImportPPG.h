//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_ImportPPG_h
#define pybind_smtk_session_polygon_operators_ImportPPG_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/ImportPPG.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::ImportPPG > pybind11_init_smtk_session_polygon_ImportPPG(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::ImportPPG > instance(m, "ImportPPG", parent);
  instance
    .def("ableToOperate", &smtk::session::polygon::ImportPPG::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ImportPPG> (*)()) &smtk::session::polygon::ImportPPG::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::ImportPPG> (*)(::std::shared_ptr<smtk::session::polygon::ImportPPG> &)) &smtk::session::polygon::ImportPPG::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::ImportPPG> (smtk::session::polygon::ImportPPG::*)() const) &smtk::session::polygon::ImportPPG::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::ImportPPG> (smtk::session::polygon::ImportPPG::*)()) &smtk::session::polygon::ImportPPG::shared_from_this)
    ;
  return instance;
}

#endif
