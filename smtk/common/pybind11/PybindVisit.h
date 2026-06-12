//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_pybind_PybindVisit_h
#define smtk_common_pybind_PybindVisit_h

#include "smtk/common/Visit.h"

inline std::pair<py::enum_<smtk::common::Visit>, py::enum_<smtk::common::Visited>>
pybind11_init_smtk_common_Visit(py::module& common)
{
  py::enum_<smtk::common::Visit> visit(common, "Visit");
  visit
    .value("Continue", smtk::common::Visit::Continue)
    .value("Halt", smtk::common::Visit::Halt)
    ;

  py::enum_<smtk::common::Visited> visited(common, "Visited");
  visited
    .value("All", smtk::common::Visited::All)
    .value("Some", smtk::common::Visited::Some)
    .value("Empty", smtk::common::Visited::Empty)
    ;

  return {visit, visited};
}

#endif // smtk_common_pybind_PybindVisit_h
