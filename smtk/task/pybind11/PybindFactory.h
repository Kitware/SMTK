//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_task_Factory_h
#define pybind_smtk_task_Factory_h

#include <pybind11/pybind11.h>

#include "smtk/task/Factory.h"

namespace py = pybind11;

inline py::class_<
  smtk::task::Factory
  // , smtk::common::Factory<
  //   smtk::task::Task,
  //   void,
  //   std::tuple<nlohmann::json, std::shared_ptr<smtk::common::Managers> >,
  //   std::tuple<nlohmann::json, std::set<std::shared_ptr<smtk::task::Task>>, std::shared_ptr<smtk::common::Managers> >
  // >
> pybind11_init_smtk_task_Factory(py::module &m)
{
  py::class_<
    smtk::task::Factory
    // , smtk::common::Factory<
    //   smtk::task::Task,
    //   void,
    //   std::tuple<nlohmann::json, std::shared_ptr<smtk::common::Managers> >,
    //   std::tuple<nlohmann::json, std::set<std::shared_ptr<smtk::task::Task>>, std::shared_ptr<smtk::common::Managers> >
    // >
  > instance(m, "Factory");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::task::Factory const &>())
    .def("deepcopy", (smtk::task::Factory & (smtk::task::Factory::*)(::smtk::task::Factory const &)) &smtk::task::Factory::operator=)
    .def("createFromName", [](const smtk::task::Factory& factory, const std::string& name) {
      std::shared_ptr<smtk::task::Task> object(factory.createFromName(name));
      return object;
      })
  /*
    .def("createFromName", [](const smtk::task::Factory& factory, const std::string& name, const std::string& json, const std::shared_ptr<smtk::common::Managers>& managers) {
      smtk::task::Task::Configuration config(json);
      std::shared_ptr<smtk::task::Task> object = factory.createFromName(name, config, managers);
      return object;
      })
      */
    ;
  return instance;
}

#endif
