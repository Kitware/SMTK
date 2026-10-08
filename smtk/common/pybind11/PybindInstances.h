//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Instances_h
#define pybind_smtk_common_Instances_h

#include "smtk/common/Instances.h"

template<typename ObjectType, typename... InputTypes>
py::class_<smtk::common::Instances<ObjectType, InputTypes...>> pybind11_init_smtk_common_Instances(
  py::module module, const std::string& instanceTypeName)
{
  using InstanceType = smtk::common::Instances<ObjectType, InputTypes...>;
  py::class_<smtk::common::Instances<ObjectType, InputTypes...>> instance(module, instanceTypeName.c_str());
  instance
    .def("manage", [](InstanceType& self, const std::shared_ptr<ObjectType>& instance)
      { return self.manage(instance); }, py::arg("instance"))
    .def("unmanage", [](InstanceType& self, const std::shared_ptr<ObjectType>& instance)
      { return self.unmanage(instance); }, py::arg("instance"))
    .def("contains", [](InstanceType& self, const std::shared_ptr<ObjectType>& instance)
      { return self.contains(instance); }, py::arg("instance"))
    .def("clear", &InstanceType::clear)
    .def("size", &InstanceType::size)
    ;

  return instance;
}

#endif // pybind_smtk_common_Instances_h
