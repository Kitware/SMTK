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


#include "smtk/model/pybind11/PyOperator.h"

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
#include "smtk/mesh/core/Manager.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

namespace py = pybind11;

void pybind11_init_smtk_model_OperatorOutcome(py::module &m)
{
  py::enum_<smtk::operation::NewOpOutcome>(m, "OperatorOutcome")
    .value("UNABLE_TO_OPERATE", smtk::operation::NewOpOutcome::UNABLE_TO_OPERATE)
    .value("OPERATION_CANCELED", smtk::operation::NewOpOutcome::OPERATION_CANCELED)
    .value("OPERATION_FAILED", smtk::operation::NewOpOutcome::OPERATION_FAILED)
    .value("OPERATION_SUCCEEDED", smtk::operation::NewOpOutcome::OPERATION_SUCCEEDED)
    .value("OUTCOME_UNKNOWN", smtk::operation::NewOpOutcome::OUTCOME_UNKNOWN)
    .export_values();
}

PySharedPtrClass< smtk::operation::NewOp, smtk::model::PyOperator > pybind11_init_smtk_model_Operator(py::module &m)
{
  typedef std::underlying_type<::smtk::attribute::SearchStyle>::type SearchStyleType;

  PySharedPtrClass< smtk::operation::NewOp, smtk::model::PyOperator > instance(m, "Operator");
  instance
    .def(py::init<>())
    .def("__lt__", (bool (smtk::operation::NewOp::*)(::smtk::model::Operator const &) const) &smtk::model::Operator::operator<)
    .def("deepcopy", (smtk::operation::NewOp & (smtk::model::Operator::*)(::smtk::model::Operator const &)) &smtk::model::Operator::operator=)
    .def("ableToOperate", &smtk::operation::NewOp::ableToOperate)
    .def("associateEntity", &smtk::operation::NewOp::associateEntity, py::arg("entity"))
    .def("className", &smtk::operation::NewOp::className)
    .def("classname", &smtk::operation::NewOp::classname)
    .def_static("create", &smtk::model::PyOperator::create)
    .def("createResult", &smtk::operation::NewOp::createResult, py::arg("outcome") = ::smtk::model::OperatorOutcome::UNABLE_TO_OPERATE)
    .def("definition", &smtk::operation::NewOp::definition)
    .def("disassociateEntity", &smtk::operation::NewOp::disassociateEntity, py::arg("entity"))
    .def("ensureSpecification", &smtk::operation::NewOp::ensureSpecification)
    .def("eraseResult", &smtk::operation::NewOp::eraseResult, py::arg("res"))
    .def("_find", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findAs<smtk::attribute::Item>(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findDirectory", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findDirectory(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findDouble", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findDouble(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findFile", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findFile(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findGroup", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findGroup(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findInt", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findInt(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMesh", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findMesh(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMeshSelection", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findMeshSelection(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findModelEntity", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findModelEntity(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findRef", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findRef(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findString", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findString(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findVoid", [](smtk::operation::NewOp& o, const std::string& s, SearchStyleType i) { return o.findVoid(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("log", &smtk::operation::NewOp::log)
    .def("manager", &smtk::operation::NewOp::manager)
    .def("meshManager", &smtk::operation::NewOp::meshManager)
    .def("name", &smtk::operation::NewOp::name)
    .def("observe", (void (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorWithResultCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("operate", &smtk::operation::NewOp::operate)
    .def("removeAllAssociations", &smtk::operation::NewOp::removeAllAssociations)
    .def("associatedEntities", [](smtk::operation::NewOp& o){ return o.associatedEntitiesAs<smtk::model::EntityRefs>(); })
    .def("session", &smtk::operation::NewOp::session)
    .def("setManager", &smtk::operation::NewOp::setManager, py::arg("manager"))
    .def("setMeshManager", &smtk::operation::NewOp::setMeshManager, py::arg("s"))
    .def("setResultOutcome", &smtk::operation::NewOp::setResultOutcome, py::arg("res"), py::arg("outcome"))
    .def("setSession", &smtk::operation::NewOp::setSession, py::arg("session"))
    .def("setSpecification", &smtk::operation::NewOp::setSpecification, py::arg("spec"))
    .def("specification", &smtk::operation::NewOp::specification)
    .def("trigger", (int (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType)) &smtk::model::Operator::trigger, py::arg("event"))
    .def("trigger", (int (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorResult const &)) &smtk::model::Operator::trigger, py::arg("event"), py::arg("result"))
    .def("unobserve", (void (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType, ::smtk::model::BareOperatorCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::operation::NewOp::*)(::smtk::model::OperatorEventType, ::smtk::model::OperatorWithResultCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("findAsInt", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::IntItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDouble", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DoubleItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsString", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::StringItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsFile", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::FileItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDirectory", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DirectoryItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsGroup", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::GroupItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsRef", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::RefItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsModelEntity", [](const smtk::operation::NewOp& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::ModelEntityItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    ;
  py::enum_<smtk::operation::NewOp::ResultEntityOrigin>(instance, "ResultEntityOrigin")
    .value("CREATED", smtk::operation::NewOp::ResultEntityOrigin::CREATED)
    .value("MODIFIED", smtk::operation::NewOp::ResultEntityOrigin::MODIFIED)
    .value("EXPUNGED", smtk::operation::NewOp::ResultEntityOrigin::EXPUNGED)
    .value("UNKNOWN", smtk::operation::NewOp::ResultEntityOrigin::UNKNOWN)
    .export_values();

  py::implicitly_convertible<py::object, smtk::operation::NewOp>();

  return instance;
}

#endif
