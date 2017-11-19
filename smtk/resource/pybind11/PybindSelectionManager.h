//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_resource_SelectionManager_h
#define pybind_smtk_resource_SelectionManager_h

#include <pybind11/pybind11.h>

#include "smtk/resource/SelectionManager.h"

namespace py = pybind11;

void pybind11_init_smtk_resource_SelectionAction(py::module &m)
{
  py::enum_<smtk::resource::SelectionAction>(m, "SelectionAction")
    .value("FILTERED_REPLACE", smtk::resource::SelectionAction::FILTERED_REPLACE)
    .value("UNFILTERED_REPLACE", smtk::resource::SelectionAction::UNFILTERED_REPLACE)
    .value("FILTERED_ADD", smtk::resource::SelectionAction::FILTERED_ADD)
    .value("UNFILTERED_ADD", smtk::resource::SelectionAction::UNFILTERED_ADD)
    .value("FILTERED_SUBTRACT", smtk::resource::SelectionAction::FILTERED_SUBTRACT)
    .value("UNFILTERED_SUBTRACT", smtk::resource::SelectionAction::UNFILTERED_SUBTRACT)
    .value("DEFAULT", smtk::resource::SelectionAction::DEFAULT)
    .export_values();
}

PySharedPtrClass< smtk::resource::SelectionManager > pybind11_init_smtk_resource_SelectionManager(py::module &m)
{
  PySharedPtrClass< smtk::resource::SelectionManager > instance(m, "SelectionManager");
  instance
    .def("classname", &smtk::resource::SelectionManager::classname)
    .def_static("create", (std::shared_ptr<smtk::resource::SelectionManager> (*)()) &smtk::resource::SelectionManager::create)
    .def_static("create", (std::shared_ptr<smtk::resource::SelectionManager> (*)(::std::shared_ptr<smtk::resource::SelectionManager> &)) &smtk::resource::SelectionManager::create, py::arg("ref"))
    .def_static("instance", (std::shared_ptr<smtk::resource::SelectionManager> (*)()) &smtk::resource::SelectionManager::instance)
    .def("registerSelectionSource", &smtk::resource::SelectionManager::registerSelectionSource, py::arg("name"))
    .def("unregisterSelectionSource", &smtk::resource::SelectionManager::unregisterSelectionSource, py::arg("name"))
    .def("getSelectionSources", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const & (smtk::resource::SelectionManager::*)() const) &smtk::resource::SelectionManager::getSelectionSources)
    .def("getSelectionSources", (void (smtk::resource::SelectionManager::*)(::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > &) const) &smtk::resource::SelectionManager::getSelectionSources, py::arg("selectionSources"))
    .def("registerSelectionValue", &smtk::resource::SelectionManager::registerSelectionValue, py::arg("valueLabel"), py::arg("value"), py::arg("valueMustBeUnique") = true)
    .def("unregisterSelectionValue", (bool (smtk::resource::SelectionManager::*)(::std::string const &)) &smtk::resource::SelectionManager::unregisterSelectionValue, py::arg("valueLabel"))
    .def("unregisterSelectionValue", (bool (smtk::resource::SelectionManager::*)(int)) &smtk::resource::SelectionManager::unregisterSelectionValue, py::arg("value"))
    .def("selectionValueLabels", &smtk::resource::SelectionManager::selectionValueLabels)
    .def("selectionValueFromLabel", &smtk::resource::SelectionManager::selectionValueFromLabel, py::arg("label"))
    .def("findOrCreateLabeledValue", &smtk::resource::SelectionManager::findOrCreateLabeledValue, py::arg("label"))
    .def("setDefaultAction", &smtk::resource::SelectionManager::setDefaultAction, py::arg("action"))
    .def("defaultAction", &smtk::resource::SelectionManager::defaultAction)
    .def("setDefaultActionToReplace", &smtk::resource::SelectionManager::setDefaultActionToReplace)
    .def("setDefaultActionToAddition", &smtk::resource::SelectionManager::setDefaultActionToAddition)
    .def("setDefaultActionToSubtraction", &smtk::resource::SelectionManager::setDefaultActionToSubtraction)
    .def("modifySelection", (bool
        (smtk::resource::SelectionManager::*)(const ::std::vector<smtk::resource::Component::Ptr,
          std::allocator<smtk::resource::Component::Ptr> >&, const std::string&, int,
          smtk::resource::SelectionAction))
      &smtk::resource::SelectionManager::modifySelection)
    .def("visitSelection", &smtk::resource::SelectionManager::visitSelection, py::arg("visitor"))
    .def("listenToSelectionEvents", &smtk::resource::SelectionManager::listenToSelectionEvents, py::arg("fn"), py::arg("immediatelyNotify") = false)
    .def("unlisten", &smtk::resource::SelectionManager::unlisten, py::arg("handle"))
    .def("setFilter", &smtk::resource::SelectionManager::setFilter, py::arg("fn"), py::arg("refilterSelection") = true)
    .def("currentSelection", (const ::std::map<smtk::resource::ComponentPtr, int>&
        (smtk::resource::SelectionManager::*)() const)
      &smtk::resource::SelectionManager::currentSelection)
    ;
  return instance;
}

#endif
