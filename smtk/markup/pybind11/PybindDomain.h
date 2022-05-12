//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_markup_Domain_h
#define pybind_smtk_markup_Domain_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/markup/Domain.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::markup::Domain> pybind11_init_smtk_markup_Domain(py::module &m)
{
  PySharedPtrClass<smtk::markup::Domain> instance(m, "Domain");
  instance
    .def("name", &smtk::markup::Domain::name)
    ;
  return instance;
}

#endif
