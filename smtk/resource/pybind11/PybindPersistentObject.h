//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_PersistentObject_h
#define pybind_smtk_resource_PersistentObject_h

#include <pybind11/pybind11.h>

#include "smtk/resource/PersistentObject.h"

#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

#include <sstream>
#include <unordered_set>

namespace py = pybind11;

namespace
{
  std::unordered_set<const smtk::resource::PersistentObject*> allObjects;
}

inline PySharedPtrClass< smtk::resource::PersistentObject > pybind11_init_smtk_resource_PersistentObject(py::module &m)
{
  PySharedPtrClass< smtk::resource::PersistentObject > instance(m, "PersistentObject");
  instance
    .def("__enter__", [](smtk::resource::PersistentObject& self)
      {
        return self.shared_from_this();
      }, "Enter the runtime context related to this object."
    )
    .def("__exit__", [](smtk::resource::PersistentObject& self,
        const std::optional<pybind11::type>& exc_type,
        const std::optional<pybind11::object>& exc_value,
        const std::optional<pybind11::object>& traceback)
      {
        (void)self;
        (void)exc_type;
        (void)exc_value;
        (void)traceback;
      }, "Exit the runtime context related to this object."
    )
    .def("deepcopy", (smtk::resource::PersistentObject & (smtk::resource::PersistentObject::*)(::smtk::resource::PersistentObject const &)) &smtk::resource::PersistentObject::operator=)
    .def("typeName", &smtk::resource::PersistentObject::typeName)
    .def("id", &smtk::resource::PersistentObject::id)
    .def("setId", &smtk::resource::PersistentObject::setId, py::arg("myID"))
    .def("name", &smtk::resource::PersistentObject::name)
    .def("parentResource", &smtk::resource::PersistentObject::parentResource)
    .def("properties", (smtk::resource::Properties& (smtk::resource::PersistentObject::*)())&smtk::resource::PersistentObject::properties, py::return_value_policy::reference_internal)
    .def("typeToken", &smtk::resource::PersistentObject::typeToken)
    .def("classHierarchy", &smtk::resource::PersistentObject::classHierarchy)
    .def("matchesType", &smtk::resource::PersistentObject::matchesType, py::arg("candidate"))
    .def("generationsFromBase", &smtk::resource::PersistentObject::generationsFromBase, py::arg("base"))
    .def("pointer", [](const smtk::resource::PersistentObject& self)
      {
        std::ostringstream addr;
        addr << std::hex << &self;
        allObjects.insert(&self);
        return addr.str();
      })
    .def_static("fromPointer", [](const std::string& ptrStr)
      {
        char* end = const_cast<char*>(ptrStr.c_str() + ptrStr.size());
        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        auto* ptr = reinterpret_cast<smtk::resource::PersistentObject*>(strtoull(ptrStr.c_str(), &end, 16));
        if (ptr && allObjects.find(ptr) != allObjects.end())
        {
          return ptr->shared_from_this();
        }
        return smtk::resource::PersistentObject::Ptr();
      })
    ;
  return instance;
}

#endif
