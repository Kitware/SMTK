//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_SearchStyle_h
#define pybind_smtk_attribute_SearchStyle_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/SearchStyle.h"

namespace py = pybind11;

void pybind11_init_smtk_attribute_SearchStyle(py::module &m)
{
  py::enum_<smtk::attribute::SearchStyle>(m, "SearchStyle")
    .value("NO_CHILDREN", smtk::attribute::SearchStyle::NO_CHILDREN)
    .value("ACTIVE_CHILDREN", smtk::attribute::SearchStyle::ACTIVE_CHILDREN)
    .value("ALL_CHILDREN", smtk::attribute::SearchStyle::ALL_CHILDREN)
    .export_values();
}

#endif
