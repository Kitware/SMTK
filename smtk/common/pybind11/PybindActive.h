//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Active_h
#define pybind_smtk_common_Active_h

#include "smtk/common/Active.h"

template<typename InstancesType, typename ObjectType = typename InstancesType::ObjectType>
py::class_<smtk::common::Active<InstancesType, ObjectType>> pybind11_init_smtk_common_Active(
  py::module module, const std::string& activeTypeName)
{
  using ActiveType = smtk::common::Active<InstancesType, ObjectType>;
  py::class_<ActiveType> instance(module, activeTypeName.c_str());
  instance
    .def("object", &ActiveType::object)
    .def("switchTo", &ActiveType::switchTo, py::arg("object"))
    ;
  return instance;
}

#endif // pybind_smtk_common_Instances_h
