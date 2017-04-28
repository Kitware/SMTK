//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_System_h
#define pybind_smtk_attribute_System_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/System.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/common/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/View.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::System, smtk::common::Resource > pybind11_init_smtk_attribute_System(py::module &m)
{
  PySharedPtrClass< smtk::attribute::System, smtk::common::Resource > instance(m, "System");
  instance
    .def("deepcopy", (smtk::attribute::System & (smtk::attribute::System::*)(::smtk::attribute::System const &)) &smtk::attribute::System::operator=)
    .def("addAdvanceLevel", &smtk::attribute::System::addAdvanceLevel, py::arg("level"), py::arg("label"), py::arg("l_color") = 0)
    .def("addView", &smtk::attribute::System::addView, py::arg("arg0"))
    .def("advanceLevelColor", &smtk::attribute::System::advanceLevelColor, py::arg("level"))
    .def("advanceLevels", &smtk::attribute::System::advanceLevels)
    .def("analyses", &smtk::attribute::System::analyses)
    .def("analysisCategories", &smtk::attribute::System::analysisCategories, py::arg("analysisType"))
    .def("attributes", &smtk::attribute::System::attributes, py::arg("result"))
    .def("attributes", [](const smtk::attribute::System& sys){ std::vector<smtk::attribute::AttributePtr> result; sys.attributes(result); return result; })
    .def("categories", &smtk::attribute::System::categories)
    .def("copyAttribute", &smtk::attribute::System::copyAttribute, py::arg("att"), py::arg("copyModelAssociations") = false, py::arg("options") = 0)
    .def("copyDefinition", &smtk::attribute::System::copyDefinition, py::arg("def"), py::arg("options") = 0)
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &, ::std::string const &)) &smtk::attribute::System::createAttribute, py::arg("name"), py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::smtk::attribute::DefinitionPtr)) &smtk::attribute::System::createAttribute, py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &)) &smtk::attribute::System::createAttribute, py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::System::createAttribute, py::arg("name"), py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &, ::std::string const &, ::smtk::common::UUID const &)) &smtk::attribute::System::createAttribute, py::arg("name"), py::arg("type"), py::arg("id"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &, ::smtk::attribute::DefinitionPtr, ::smtk::common::UUID const &)) &smtk::attribute::System::createAttribute, py::arg("name"), py::arg("def"), py::arg("id"))
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::System::*)(::std::string const &, ::std::string const &)) &smtk::attribute::System::createDefinition, py::arg("typeName"), py::arg("baseTypeName") = "")
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::System::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::System::createDefinition, py::arg("name"), py::arg("baseDefiniiton"))
    .def("createUniqueName", &smtk::attribute::System::createUniqueName, py::arg("type"))
    .def("defineAnalysis", &smtk::attribute::System::defineAnalysis, py::arg("analysisName"), py::arg("categories"))
    .def("definitions", &smtk::attribute::System::definitions, py::arg("result"))
    .def("definitions", [](const smtk::attribute::System& sys){ std::vector<smtk::attribute::DefinitionPtr> result; sys.definitions(result); return result; })
    .def("derivedDefinitions", &smtk::attribute::System::derivedDefinitions, py::arg("def"), py::arg("result"))
    .def("findAllDerivedDefinitions", &smtk::attribute::System::findAllDerivedDefinitions, py::arg("def"), py::arg("concreteOnly"), py::arg("result"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::std::string const &) const) &smtk::attribute::System::findAttribute, py::arg("name"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::System::*)(::smtk::common::UUID const &) const) &smtk::attribute::System::findAttribute, py::arg("id"))
    .def("findAttributes", (void (smtk::attribute::System::*)(::std::string const &, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::System::findAttributes, py::arg("type"), py::arg("result"))
    .def("findAttributes", (std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > (smtk::attribute::System::*)(::std::string const &) const) &smtk::attribute::System::findAttributes, py::arg("type"))
    .def("findAttributes", (void (smtk::attribute::System::*)(::smtk::attribute::DefinitionPtr, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::System::findAttributes, py::arg("def"), py::arg("result"))
    .def("findBaseDefinitions", &smtk::attribute::System::findBaseDefinitions, py::arg("result"))
    .def("findDefinition", &smtk::attribute::System::findDefinition, py::arg("type"))
    .def("findDefinitionAttributes", &smtk::attribute::System::findDefinitionAttributes, py::arg("type"), py::arg("result"))
    .def("findDefinitions", &smtk::attribute::System::findDefinitions, py::arg("mask"), py::arg("result"))
    .def("findIsUniqueBaseClass", &smtk::attribute::System::findIsUniqueBaseClass, py::arg("attDef"))
    .def("findTopLevelView", &smtk::attribute::System::findTopLevelView)
    .def("findTopLevelViews", &smtk::attribute::System::findTopLevelViews)
    .def("findView", &smtk::attribute::System::findView, py::arg("title"))
    .def("findViewByType", &smtk::attribute::System::findViewByType, py::arg("vtype"))
    .def("hasAttributes", &smtk::attribute::System::hasAttributes)
    .def("numberOfAdvanceLevels", &smtk::attribute::System::numberOfAdvanceLevels)
    .def("numberOfAnalyses", &smtk::attribute::System::numberOfAnalyses)
    .def("numberOfCategories", &smtk::attribute::System::numberOfCategories)
    .def("refModelManager", &smtk::attribute::System::refModelManager)
    .def("removeAttribute", &smtk::attribute::System::removeAttribute, py::arg("att"))
    .def("rename", &smtk::attribute::System::rename, py::arg("att"), py::arg("newName"))
    .def("resourceType", &smtk::attribute::System::resourceType)
    .def("setAdvanceLevelColor", &smtk::attribute::System::setAdvanceLevelColor, py::arg("level"), py::arg("l_color"))
    .def("setRefModelManager", &smtk::attribute::System::setRefModelManager, py::arg("refModelMgr"))
    .def("updateCategories", &smtk::attribute::System::updateCategories)
    .def("updateDerivedDefinitionIndexOffsets", &smtk::attribute::System::updateDerivedDefinitionIndexOffsets, py::arg("def"))
    .def("views", &smtk::attribute::System::views)
    .def_static("New", [](){ return smtk::attribute::System::create(); }, py::return_value_policy::take_ownership)
    .def_static("create", [](){ return smtk::attribute::System::create(); }, py::return_value_policy::take_ownership)
    ;
  py::enum_<smtk::attribute::System::CopyOptions>(instance, "CopyOptions")
    .value("COPY_ASSOCIATIONS", smtk::attribute::System::CopyOptions::COPY_ASSOCIATIONS)
    .value("FORCE_COPY_ASSOCIATIONS", smtk::attribute::System::CopyOptions::FORCE_COPY_ASSOCIATIONS)
    .export_values();
  return instance;
}

#endif
