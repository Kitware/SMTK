//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_view_HandleManager_h
#define pybind_smtk_view_HandleManager_h

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/Item.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/HandleManager.h"

#include <sstream>
#include <unordered_set>

namespace py = pybind11;

inline py::class_<smtk::view::HandleManager::Observers::Key> pybind11_init_smtk_view_HandleManager_Observers_Key(py::module& m)
{
  py::class_<smtk::view::HandleManager::Observers::Key> instance(m, "HandleObserversKey");
  instance
    .def("assigned", &smtk::view::HandleManager::Observers::Key::assigned)
    .def("release", &smtk::view::HandleManager::Observers::Key::release)
    ;
  return instance;
}

inline py::class_<smtk::view::HandleManager::Observers> pybind11_init_smtk_view_HandleManager_Observers(py::module& m)
{
  py::class_<smtk::view::HandleManager::Observers> instance(m, "HandleObservers");
  instance
    .def("insert", [](
      smtk::view::HandleManager::Observers& observers,
      std::function<
        void(
          const smtk::view::HandleManager::HandlesByType&,
          smtk::view::HandleManager::EventType
        )
      > observer,
      smtk::view::HandleManager::Observers::Priority priority,
      const std::string& description)
      {
        return observers.insert(observer, priority, false, description);
      }, py::arg("observer"), py::arg("priority"), py::arg("description")
    )
    .def("erase", [](
      smtk::view::HandleManager::Observers& observers,
      smtk::view::HandleManager::Observers::Key& key)
      {
        observers.erase(key);
      },
      py::arg("key")
    )
    ;
  return instance;
}

inline void pybind11_init_smtk_view_HandleManager_EventType(py::module &m)
{
  py::enum_<smtk::view::HandleManager::EventType>(m, "HandleManagerEventType")
    .value("Created",  smtk::view::HandleManager::EventType::Created)
    .value("Modified", smtk::view::HandleManager::EventType::Modified)
    .value("Expunged", smtk::view::HandleManager::EventType::Expunged)
    .export_values();
}

inline PySharedPtrClass<smtk::view::HandleManager> pybind11_init_smtk_view_HandleManager(py::module &m)
{
  PySharedPtrClass< smtk::view::HandleManager > instance(m, "HandleManager"); // , py::module_local());
  pybind11_init_smtk_view_HandleManager_EventType(m);
  instance
    .def_static("instance", &smtk::view::HandleManager::instance)
    .def("handle", [](smtk::view::HandleManager& handleManager, smtk::attribute::Item* item)
      {
        return handleManager.handle(item);
      }, py::arg("item"))
    .def("handle", [](smtk::view::HandleManager& handleManager, smtk::view::Configuration* view)
      {
        return handleManager.handle(view);
      }, py::arg("view"))
    .def("handle", [](smtk::view::HandleManager& handleManager, smtk::resource::PersistentObject* object)
      {
        return handleManager.handle(object);
      }, py::arg("object"))
    .def("item", [](smtk::view::HandleManager& handleManager, const std::string& ptrStr)
      {
        auto* item = handleManager.fromHandle<smtk::attribute::Item>(ptrStr);
        if (!item)
        {
          return smtk::attribute::Item::Ptr();
        }
        return item->shared_from_this();
      })
    .def("view", [](smtk::view::HandleManager& handleManager, const std::string& ptrStr)
      {
        auto* view = handleManager.fromHandle<smtk::view::Configuration>(ptrStr);
        if (!view)
        {
          return smtk::view::Configuration::Ptr();
        }
        return view->shared_from_this();
      })
    .def("object", [](smtk::view::HandleManager& handleManager, const std::string& ptrStr)
      {
        auto* object = handleManager.fromHandle<smtk::resource::PersistentObject>(ptrStr);
        if (!object)
        {
          return smtk::resource::PersistentObject::Ptr();
        }
        return object->shared_from_this();
      })
    .def("observers", [](smtk::view::HandleManager& handleManager)
      {
        return &handleManager.observers();
      }, py::return_value_policy::reference_internal
    )
    ;
  return instance;
}

#endif
