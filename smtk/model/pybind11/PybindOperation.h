//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Operation_h
#define pybind_smtk_model_Operation_h

#include <pybind11/pybind11.h>

#include <type_traits>


#include "smtk/model/pybind11/PyOperation.h"

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
#include "smtk/model/Session.h"

namespace py = pybind11;

void pybind11_init_smtk_model_OperationOutcome(py::module &m)
{
  py::enum_<smtk::operation::OperationOutcome>(m, "OperationOutcome")
    .value("UNABLE_TO_OPERATE", smtk::operation::OperationOutcome::UNABLE_TO_OPERATE)
    .value("OPERATION_CANCELED", smtk::operation::OperationOutcome::OPERATION_CANCELED)
    .value("OPERATION_FAILED", smtk::operation::OperationOutcome::OPERATION_FAILED)
    .value("OPERATION_SUCCEEDED", smtk::operation::OperationOutcome::OPERATION_SUCCEEDED)
    .value("OUTCOME_UNKNOWN", smtk::operation::OperationOutcome::OUTCOME_UNKNOWN)
    .export_values();
}

PySharedPtrClass< smtk::operation::Operation, smtk::model::PyOperation > pybind11_init_smtk_model_Operation(py::module &m)
{
  typedef std::underlying_type<::smtk::attribute::SearchStyle>::type SearchStyleType;

  PySharedPtrClass< smtk::operation::Operation, smtk::model::PyOperation > instance(m, "Operation");
  instance
    .def(py::init<>())
    .def("__lt__", (bool (smtk::operation::Operation::*)(::smtk::model::Operation const &) const) &smtk::model::Operation::operator<)
    .def("deepcopy", (smtk::operation::Operation & (smtk::model::Operation::*)(::smtk::model::Operation const &)) &smtk::model::Operator::operator=)
    .def("ableToOperate", &smtk::operation::Operation::ableToOperate)
    .def("associateEntity", &smtk::operation::Operation::associateEntity, py::arg("entity"))
    .def_static("create", &smtk::model::PyOperation::create)
    .def("createResult", &smtk::operation::Operation::createResult, py::arg("outcome") = ::smtk::model::OperationOutcome::UNABLE_TO_OPERATE)
    .def("definition", &smtk::operation::Operation::definition)
    .def("disassociateEntity", &smtk::operation::Operation::disassociateEntity, py::arg("entity"))
    .def("ensureSpecification", &smtk::operation::Operation::ensureSpecification)
    .def("eraseResult", &smtk::operation::Operation::eraseResult, py::arg("res"))
    .def("_find", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findAs<smtk::attribute::Item>(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findDirectory", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findDirectory(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findDouble", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findDouble(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findFile", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findFile(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findGroup", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findGroup(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findInt", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findInt(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMesh", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findMesh(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findMeshSelection", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findMeshSelection(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findModelEntity", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findModelEntity(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findRef", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findRef(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findString", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findString(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("findVoid", [](smtk::operation::Operation& o, const std::string& s, SearchStyleType i) { return o.findVoid(s, ::smtk::attribute::SearchStyle(i)); },
         py::arg("name"), py::arg("style") = static_cast<SearchStyleType>(::smtk::attribute::SearchStyle::ALL_CHILDREN))
    .def("log", &smtk::operation::Operation::log)
    .def("manager", &smtk::operation::Operation::manager)
    .def("meshManager", &smtk::operation::Operation::meshManager)
    .def("name", &smtk::operation::Operation::name)
    .def("observe", (void (smtk::operation::Operation::*)(::smtk::model::OperationEventType, ::smtk::model::BareOperationCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("observe", (void (smtk::operation::Operation::*)(::smtk::model::OperationEventType, ::smtk::model::OperationWithResultCallback, void *)) &smtk::model::Operator::observe, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("operate", &smtk::operation::Operation::operate)
    .def("removeAllAssociations", &smtk::operation::Operation::removeAllAssociations)
    .def("associatedEntities", [](smtk::operation::Operation& o){ return o.associatedEntitiesAs<smtk::model::EntityRefs>(); })
    .def("session", &smtk::operation::Operation::session)
    .def("setManager", &smtk::operation::Operation::setManager, py::arg("manager"))
    .def("setMeshManager", &smtk::operation::Operation::setMeshManager, py::arg("s"))
    .def("setResultOutcome", &smtk::operation::Operation::setResultOutcome, py::arg("res"), py::arg("outcome"))
    .def("setSession", &smtk::operation::Operation::setSession, py::arg("session"))
    .def("setSpecification", &smtk::operation::Operation::setSpecification, py::arg("spec"))
    .def("specification", &smtk::operation::Operation::specification)
    .def("trigger", (int (smtk::operation::Operation::*)(::smtk::model::OperationEventType)) &smtk::model::Operation::trigger, py::arg("event"))
    .def("trigger", (int (smtk::operation::Operation::*)(::smtk::model::OperationEventType, ::smtk::model::OperationResult const &)) &smtk::model::Operator::trigger, py::arg("event"), py::arg("result"))
    .def("unobserve", (void (smtk::operation::Operation::*)(::smtk::model::OperationEventType, ::smtk::model::BareOperationCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("unobserve", (void (smtk::operation::Operation::*)(::smtk::model::OperationEventType, ::smtk::model::OperationWithResultCallback, void *)) &smtk::model::Operator::unobserve, py::arg("event"), py::arg("functionHandle"), py::arg("callData"))
    .def("findAsInt", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::IntItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDouble", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DoubleItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsString", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::StringItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsFile", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::FileItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsDirectory", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::DirectoryItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsGroup", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::GroupItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsRef", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::RefItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    .def("findAsModelEntity", [](const smtk::operation::Operation& o, const std::string& s) {
        return o.specification()->findAs<smtk::attribute::ModelEntityItem>(s, smtk::attribute::SearchStyle::ALL_CHILDREN); })
    ;
  py::enum_<smtk::operation::Operation::ResultEntityOrigin>(instance, "ResultEntityOrigin")
    .value("CREATED", smtk::operation::Operation::ResultEntityOrigin::CREATED)
    .value("MODIFIED", smtk::operation::Operation::ResultEntityOrigin::MODIFIED)
    .value("EXPUNGED", smtk::operation::Operation::ResultEntityOrigin::EXPUNGED)
    .value("UNKNOWN", smtk::operation::Operation::ResultEntityOrigin::UNKNOWN)
    .export_values();

  py::implicitly_convertible<py::object, smtk::operation::Operation>();

  return instance;
}

#endif
