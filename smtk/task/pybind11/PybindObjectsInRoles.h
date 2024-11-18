//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_ObjectsInRoles_h
#define pybind_smtk_task_ObjectsInRoles_h

#include <pybind11/pybind11.h>

#include "smtk/task/ObjectsInRoles.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::task::ObjectsInRoles > pybind11_init_smtk_task_ObjectsInRoles(py::module &m)
{
  PySharedPtrClass< smtk::task::ObjectsInRoles > instance(m, "ObjectsInRoles");
  instance
    .def("typeName", &smtk::task::ObjectsInRoles::typeName)
    .def_static("create", (std::shared_ptr<smtk::task::ObjectsInRoles> (*)()) &smtk::task::ObjectsInRoles::create)
    .def("data", &smtk::task::ObjectsInRoles::data, py::return_value_policy::reference_internal)
    .def("addObject",
      [](smtk::task::ObjectsInRoles& self,
        const smtk::resource::PersistentObjectPtr& object,
        smtk::string::Token role)
      {
        return self.addObject(object.get(), role);
      }, py::arg("object"), py::arg("role"))
    .def("removeObject",
      [](smtk::task::ObjectsInRoles& self,
        const smtk::resource::PersistentObjectPtr& object,
        smtk::string::Token role)
      {
        return self.removeObject(object.get(), role);
      }, py::arg("object"), py::arg("role") = smtk::string::Token())
    ;
  return instance;
}

#endif
