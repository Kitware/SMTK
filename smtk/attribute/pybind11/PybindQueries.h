//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Queries_h
#define pybind_smtk_attribute_Queries_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/utility/Queries.h"

namespace py = pybind11;


void pybind11_init_smtk_attribute_queries(py::module &m)
{
  m.def("checkUniquenessCondition", &smtk::attribute::utility::checkUniquenessCondition);
  m.def("associatableObjects", &smtk::attribute::utility::associatableObjects);
}

#endif
