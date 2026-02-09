//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_Managers_h
#define pybind_smtk_common_Managers_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/common/Managers.h"

#include "smtk/geometry/Manager.h"
#include "smtk/job/Manager.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/string/Token.h"
#include "smtk/task/Manager.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Selection.h"

#include <string>
#include <vector>

using namespace smtk::string::literals;  // for ""_hash


namespace py = pybind11;

inline PySharedPtrClass< smtk::common::Managers > pybind11_init_smtk_common_Managers(py::module &m)
{
  PySharedPtrClass< smtk::common::Managers > instance(m, "Managers");
  instance
    .def_static("create", (std::shared_ptr<smtk::common::Managers> (*)()) &smtk::common::Managers::create)
    .def("__enter__", [](smtk::common::Managers& self)
      {
        // Keep the instance alive for the duration of the context.
        return self.shared_from_this();
      }, "Enter the runtime context related to this object."
    )
    .def("__exit__", [](smtk::common::Managers& self,
        const std::optional<pybind11::type>& exc_type,
        const std::optional<pybind11::object>& exc_value,
        const std::optional<pybind11::object>& traceback
        )
      {
        (void)self;
        (void)exc_type;
        (void)exc_value;
        (void)traceback;
      }, "Exit the runtime context related to this object."
    )
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::operation::Manager>& operationManager)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(operationManager);
      }, py::arg("operationManager"))
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::project::Manager>& projectManager)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(projectManager);
      }, py::arg("projectManager"))
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::resource::Manager>& resourceManager)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(resourceManager);
      }, py::arg("resourceManager"))
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::task::Manager>& taskManager)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(taskManager);
      }, py::arg("taskManager"))
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::view::Manager>& viewManager)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(viewManager);
      }, py::arg("viewManager"))
    .def("insert_or_assign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::view::Selection>& viewSelection)
      {
        std::cerr << "Deprecated after 23.08; use insertOrAssign() instead.\n";
        managers.insertOrAssign(viewSelection);
      }, py::arg("selection"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::operation::Manager>& operationManager)
      {
        managers.insertOrAssign(operationManager);
      }, py::arg("operationManager"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::project::Manager>& projectManager)
      {
        managers.insertOrAssign(projectManager);
      }, py::arg("projectManager"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::resource::Manager>& resourceManager)
      {
        managers.insertOrAssign(resourceManager);
      }, py::arg("resourceManager"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::task::Manager>& taskManager)
      {
        managers.insertOrAssign(taskManager);
      }, py::arg("taskManager"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::view::Manager>& viewManager)
      {
        managers.insertOrAssign(viewManager);
      }, py::arg("viewManager"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::view::Selection>& viewSelection)
      {
        managers.insertOrAssign(viewSelection);
      }, py::arg("selection"))
    .def("insertOrAssign", [](smtk::common::Managers& managers, std::shared_ptr<smtk::job::Manager>& jobManager)
      {
        managers.insertOrAssign(jobManager);
      }, py::arg("job_manager"))
    .def("get", [](smtk::common::Managers& managers, const std::string& managerType) -> py::object
      {
        smtk::string::Token token = managerType;
        switch(token.id())
        {
          case "shared_ptr<smtk::operation::Manager>"_hash:
          case "smtk::operation::Manager"_hash:
          case "smtk.operation.Manager"_hash:
            return py::cast(managers.get<smtk::operation::Manager::Ptr>());
          case "shared_ptr<smtk::project::Manager>"_hash:
          case "smtk::project::Manager"_hash:
          case "smtk.project.Manager"_hash:
            return py::cast(managers.get<smtk::project::Manager::Ptr>());
          case "shared_ptr<smtk::resource::Manager>"_hash:
          case "smtk::resource::Manager"_hash:
          case "smtk.resource.Manager"_hash:
            return py::cast(managers.get<smtk::resource::Manager::Ptr>());
          case "shared_ptr<smtk::job::Manager>"_hash:
          case "smtk::job::Manager"_hash:
          case "smtk.job.Manager"_hash:
            return py::cast(managers.get<smtk::job::Manager::Ptr>());
          case "shared_ptr<smtk::task::Manager>"_hash:
          case "smtk::task::Manager"_hash:
          case "smtk.task.Manager"_hash:
            return py::cast(managers.get<smtk::task::Manager::Ptr>());
          case "shared_ptr<smtk::view::Manager>"_hash:
          case "smtk::view::Manager"_hash:
          case "smtk.view.Manager"_hash:
            return py::cast(managers.get<smtk::view::Manager::Ptr>());
          case "shared_ptr<smtk::view::Selection>"_hash:
          case "smtk::view::Selection"_hash:
          case "smtk.view.Selection"_hash:
            return py::cast(managers.get<smtk::view::Selection::Ptr>());
          case "shared_ptr<smtk::geometry::Manager>"_hash:
          case "smtk::geometry::Manager"_hash:
          case "smtk.geometry.Manager"_hash:
            return py::cast(managers.get<smtk::geometry::Manager::Ptr>());

          // Not handled yet:
          // case "shared_ptr<smtk::attribute::AssociationRuleManager>"_hash:
          // case "shared_ptr<smtk::attribute::EvaluatorManager>"_hash:
          // case "shared_ptr<smtk::attribute::ItemDefinitionManager>"_hash:
          // case "shared_ptr<smtk::attribute::UpdateManager>"_hash:
          // case "shared_ptr<smtk::extension::qtManager>"_hash:
          // case "shared_ptr<smtk::resource::query::Manager>"_hash:
          // case "shared_ptr<smtk::view::NameManager>"_hash:
          default:
            return py::object(py::cast(nullptr));
            // throw smtk::common::TypeContainer::BadTypeError(managerType);
        }
      }, py::arg("managerType"))
    .def("keys", [](smtk::common::Managers& managers)
      {
        std::vector<std::string> keys;
        auto tokenKeys = managers.keys();
        keys.reserve(tokenKeys.size());
        for (const auto& key : tokenKeys)
        {
          keys.push_back(key.data());
        }
        return keys;
      })
    ;
  return instance;
}

#endif
