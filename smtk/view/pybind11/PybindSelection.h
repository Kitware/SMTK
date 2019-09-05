//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_view_Selection_h
#define pybind_smtk_view_Selection_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include "smtk/view/Selection.h"

#include "smtk/resource/PersistentObject.h"

#include <vector>

namespace py = pybind11;

void pybind11_init_smtk_view_SelectionAction(py::module &m)
{
  py::enum_<smtk::view::SelectionAction>(m, "SelectionAction")
    .value("FILTERED_REPLACE", smtk::view::SelectionAction::FILTERED_REPLACE)
    .value("UNFILTERED_REPLACE", smtk::view::SelectionAction::UNFILTERED_REPLACE)
    .value("FILTERED_ADD", smtk::view::SelectionAction::FILTERED_ADD)
    .value("UNFILTERED_ADD", smtk::view::SelectionAction::UNFILTERED_ADD)
    .value("FILTERED_SUBTRACT", smtk::view::SelectionAction::FILTERED_SUBTRACT)
    .value("UNFILTERED_SUBTRACT", smtk::view::SelectionAction::UNFILTERED_SUBTRACT)
    .value("DEFAULT", smtk::view::SelectionAction::DEFAULT)
    .export_values();
}

PySharedPtrClass< smtk::view::Selection > pybind11_init_smtk_view_Selection(py::module &m)
{
  PySharedPtrClass< smtk::view::Selection > instance(m, "Selection");
  instance
    .def("typeName", &smtk::view::Selection::typeName)
    .def_static("create", (std::shared_ptr<smtk::view::Selection> (*)()) &smtk::view::Selection::create)
    .def_static("create", (std::shared_ptr<smtk::view::Selection> (*)(::std::shared_ptr<smtk::view::Selection> &)) &smtk::view::Selection::create, py::arg("ref"))
    .def_static("instance", (std::shared_ptr<smtk::view::Selection> (*)()) &smtk::view::Selection::instance)
    .def("registerSelectionSource", &smtk::view::Selection::registerSelectionSource, py::arg("name"))
    .def("unregisterSelectionSource", &smtk::view::Selection::unregisterSelectionSource, py::arg("name"))
    .def("getSelectionSources", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const & (smtk::view::Selection::*)() const) &smtk::view::Selection::getSelectionSources)
    .def("getSelectionSources", (void (smtk::view::Selection::*)(::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > &) const) &smtk::view::Selection::getSelectionSources, py::arg("selectionSources"))
    .def("registerSelectionValue", &smtk::view::Selection::registerSelectionValue, py::arg("valueLabel"), py::arg("value"), py::arg("valueMustBeUnique") = true)
    .def("unregisterSelectionValue", (bool (smtk::view::Selection::*)(::std::string const &)) &smtk::view::Selection::unregisterSelectionValue, py::arg("valueLabel"))
    .def("unregisterSelectionValue", (bool (smtk::view::Selection::*)(int)) &smtk::view::Selection::unregisterSelectionValue, py::arg("value"))
    .def("selectionValueLabels", &smtk::view::Selection::selectionValueLabels)
    .def("selectionValueFromLabel", &smtk::view::Selection::selectionValueFromLabel, py::arg("label"))
    .def("findOrCreateLabeledValue", &smtk::view::Selection::findOrCreateLabeledValue, py::arg("label"))
    .def("setDefaultAction", &smtk::view::Selection::setDefaultAction, py::arg("action"))
    .def("defaultAction", &smtk::view::Selection::defaultAction)
    .def("setDefaultActionToReplace", &smtk::view::Selection::setDefaultActionToReplace)
    .def("setDefaultActionToAddition", &smtk::view::Selection::setDefaultActionToAddition)
    .def("setDefaultActionToSubtraction", &smtk::view::Selection::setDefaultActionToSubtraction)
    .def("modifySelection", (bool
        (smtk::view::Selection::*)(const ::std::vector<smtk::resource::PersistentObject::Ptr>&,
          const std::string&, int, smtk::view::SelectionAction, bool, bool))
      &smtk::view::Selection::modifySelection)
    .def("resetSelectionBits", &smtk::view::Selection::resetSelectionBits)
    .def("visitSelection", (void (smtk::view::Selection::*)(::std::function<void (std::shared_ptr<smtk::resource::PersistentObject>, int)>)) &smtk::view::Selection::visitSelection, py::arg("visitor"))
    .def("setFilter", &smtk::view::Selection::setFilter, py::arg("fn"), py::arg("refilterSelection") = true)
    .def("currentSelection", (smtk::view::Selection::SelectionMap & (smtk::view::Selection::*)(::smtk::view::Selection::SelectionMap &) const) &smtk::view::Selection::currentSelection, py::arg("selection"))
    .def("currentSelection", (smtk::view::Selection::SelectionMap const & (smtk::view::Selection::*)() const) &smtk::view::Selection::currentSelection)
    .def("observers", (smtk::view::Selection::Observers & (smtk::view::Selection::*)()) &smtk::view::Selection::observers, pybind11::return_value_policy::reference_internal)
    ;
  return instance;
}

#endif
