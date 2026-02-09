//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_JobSpecs_h
#define pybind_smtk_operation_JobSpecs_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/operation/JobSpecs.h"

#include <vector>
#include <set>
#include <map>

namespace py = pybind11;

inline void pybind11_init_smtk_operation_JobSpecs(py::module& operation)
{
  operation
    .def("addJobSpecWithAssociations", [](
        const smtk::operation::Operation::Result& result,
        const std::vector<smtk::resource::Component::Ptr>& associations,
        const std::string& jobSpecType) -> smtk::attribute::Attribute::Ptr
      {
        return smtk::operation::addJobSpecWithAssociations(result, associations, jobSpecType);
      },
      py::arg("result"), py::arg("associations"), py::arg("job_type") = "JobSpec"
    )
  ;
}

#endif // pybind_smtk_operation_JobSpecs_h
