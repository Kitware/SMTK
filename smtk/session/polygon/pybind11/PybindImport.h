//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_operators_Import_h
#define pybind_smtk_session_polygon_operators_Import_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/operators/Import.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::polygon::Import > pybind11_init_smtk_session_polygon_Import(py::module &m, PySharedPtrClass< smtk::session::polygon::Operation, smtk::operation::XMLOperation >& parent)
{
  PySharedPtrClass< smtk::session::polygon::Import > instance(m, "Import", parent);
  instance
    .def("ableToOperate", &smtk::session::polygon::Import::ableToOperate)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Import> (*)()) &smtk::session::polygon::Import::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::Import> (*)(::std::shared_ptr<smtk::session::polygon::Import> &)) &smtk::session::polygon::Import::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::polygon::Import> (smtk::session::polygon::Import::*)() const) &smtk::session::polygon::Import::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::polygon::Import> (smtk::session::polygon::Import::*)()) &smtk::session::polygon::Import::shared_from_this)
    ;
  return instance;
}

#endif
