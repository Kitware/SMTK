//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Operator_h
#define pybind_smtk_model_Operator_h

#include <pybind11/pybind11.h>

#include <type_traits>

#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/io/Logger.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

namespace py = pybind11;

void pybind11_init_smtk_model_OperatorOutcome(py::module &m)
{
  py::enum_<smtk::model::OperatorOutcome>(m, "OperatorOutcome")
    .value("UNABLE_TO_OPERATE", smtk::model::OperatorOutcome::UNABLE_TO_OPERATE)
    .value("OPERATION_CANCELED", smtk::model::OperatorOutcome::OPERATION_CANCELED)
    .value("OPERATION_FAILED", smtk::model::OperatorOutcome::OPERATION_FAILED)
    .value("OPERATION_SUCCEEDED", smtk::model::OperatorOutcome::OPERATION_SUCCEEDED)
    .value("OUTCOME_UNKNOWN", smtk::model::OperatorOutcome::OUTCOME_UNKNOWN)
    .export_values();
}

PySharedPtrClass< smtk::model::Operator > pybind11_init_smtk_model_Operator(py::module &m)
{
  typedef std::underlying_type<::smtk::attribute::SearchStyle>::type SearchStyleType;

  PySharedPtrClass< smtk::model::Operator > instance(m, "Operator");
  instance
    .def("__lt__", (bool (smtk::model::Operator::*)(::smtk::model::Operator const &) const) &smtk::model::Operator::operator<)
    .def("deepcopy", (smtk::model::Operator & (smtk::model::Operator::*)(::smtk::model::Operator const &)) &smtk::model::Operator::operator=)
    .def("ableToOperate", &smtk::model::Operator::ableToOperate)
    .def("associateEntity", &smtk::model::Operator::associateEntity, py::arg("entity"))
    .def("className", &smtk::model::Operator::className)
    .def("classname", &smtk::model::Operator::classname)
    .def("createResult", &smtk::model::Operator::createResult, py::arg("outcome") = ::smtk::model::OperatorOutcome::UNABLE_TO_OPERATE)
    .def("definition", &smtk::model::Operator::definition)
    .def("disassociateEntity", &smtk::model::Operator::disassociateEntity, py::arg("entity"))
    .def("ensureSpecification", &smtk::model::Operator::ensureSpecification)
    .def("eraseResult", &smtk::model::Operator::eraseResult, py::arg("res"))
    .def("findDirectory", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findDirectory(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findDouble", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findDouble(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findFile", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findFile(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findGroup", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findGroup(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findInt", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findInt(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMesh", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findMesh(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMeshSelection", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findMeshSelection(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findModelEntity", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findModelEntity(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findRef", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findRef(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findString", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findString(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findVoid", [](smtk::model::Operator& o, const std::string& s, SearchStyleType i) { return o.findVoid(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("log", &smtk::model::Operator::log)
    .def("manager", &smtk::model::Operator::manager)
    .def("meshManager", &smtk::model::Operator::meshManager)
    .def("name", &smtk::model::Operator::name)
    .def("observe", (void (smtk::model::Operator::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::model::Operator::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorWithResultCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("operate", &smtk::model::Operator::operate)
    .def("removeAllAssociations", &smtk::model::Operator::removeAllAssociations)
    .def("session", &smtk::model::Operator::session)
    .def("setManager", &smtk::model::Operator::setManager, py::arg("manager"))
    .def("setMeshManager", &smtk::model::Operator::setMeshManager, py::arg("s"))
    .def("setResultOutcome", &smtk::model::Operator::setResultOutcome, py::arg("res"), py::arg("outcome"))
    .def("setSession", &smtk::model::Operator::setSession, py::arg("session"))
    .def("setSpecification", &smtk::model::Operator::setSpecification, py::arg("spec"))
    .def("specification", &smtk::model::Operator::specification)
    .def("trigger", (int (smtk::model::Operator::*)(::smtk::model::OperatorEventType)) &smtk::model::Operator::trigger, py::arg("event"))
    .def("trigger", (int (smtk::model::Operator::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorResult const &)) &smtk::model::Operator::trigger, py::arg("event"), py::arg("result"))
    .def("unobserve", (void (smtk::model::Operator::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::model::Operator::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorWithResultCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("findAsInt", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::IntItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDouble", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DoubleItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsString", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::StringItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsFile", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::FileItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDirectory", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DirectoryItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsGroup", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::GroupItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsRef", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::RefItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsModelEntity", [](const smtk::model::Operator& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::ModelEntityItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    ;
  py::enum_<smtk::model::Operator::ResultEntityOrigin>(instance, "ResultEntityOrigin")
    .value("CREATED", smtk::model::Operator::ResultEntityOrigin::CREATED)
    .value("MODIFIED", smtk::model::Operator::ResultEntityOrigin::MODIFIED)
    .value("EXPUNGED", smtk::model::Operator::ResultEntityOrigin::EXPUNGED)
    .value("UNKNOWN", smtk::model::Operator::ResultEntityOrigin::UNKNOWN)
    .export_values();
  return instance;
}

void pybind11_init_smtk_model_outcomeAsString(py::module &m)
{
  m.def("outcomeAsString", &smtk::model::outcomeAsString, "", py::arg("oc"));
}

void pybind11_init_smtk_model_stringToOutcome(py::module &m)
{
  m.def("stringToOutcome", &smtk::model::stringToOutcome, "", py::arg("oc"));
}

#endif
