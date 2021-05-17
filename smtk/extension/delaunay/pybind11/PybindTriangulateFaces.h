//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_delaunay_TriangulateFaces_h
#define pybind_smtk_extension_delaunay_TriangulateFaces_h

#include <pybind11/pybind11.h>

#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

#include "smtk/operation/XMLOperation.h"

inline PySharedPtrClass< smtk::extension::delaunay::TriangulateFaces, smtk::operation::XMLOperation > pybind11_init_smtk_extension_delaunay_TriangulateFaces(py::module &m)
{
  PySharedPtrClass< smtk::extension::delaunay::TriangulateFaces, smtk::operation::XMLOperation > instance(m, "TriangulateFaces");
  instance
    .def_static("create", (std::shared_ptr<smtk::extension::delaunay::TriangulateFaces> (*)()) &smtk::extension::delaunay::TriangulateFaces::create)
    .def_static("create", (std::shared_ptr<smtk::extension::delaunay::TriangulateFaces> (*)(::std::shared_ptr<smtk::extension::delaunay::TriangulateFaces> &)) &smtk::extension::delaunay::TriangulateFaces::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::extension::delaunay::TriangulateFaces> (smtk::extension::delaunay::TriangulateFaces::*)()) &smtk::extension::delaunay::TriangulateFaces::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::extension::delaunay::TriangulateFaces> (smtk::extension::delaunay::TriangulateFaces::*)() const) &smtk::extension::delaunay::TriangulateFaces::shared_from_this)
    .def("ableToOperate", &smtk::extension::delaunay::TriangulateFaces::ableToOperate)
    ;
  return instance;
}

#endif
