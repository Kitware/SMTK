//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Collection_h
#define pybind_smtk_attribute_Collection_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Collection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/view/View.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::Collection, smtk::resource::Resource > pybind11_init_smtk_attribute_Collection(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Collection, smtk::resource::Resource > instance(m, "Collection");
  instance
    .def("addAdvanceLevel", &smtk::attribute::Collection::addAdvanceLevel, py::arg("level"), py::arg("label"), py::arg("l_color") = 0)
    .def("addView", &smtk::attribute::Collection::addView, py::arg("arg0"))
    .def("advanceLevelColor", &smtk::attribute::Collection::advanceLevelColor, py::arg("level"))
    .def("advanceLevels", &smtk::attribute::Collection::advanceLevels)
    .def("analyses", &smtk::attribute::Collection::analyses)
    .def("analysisCategories", &smtk::attribute::Collection::analysisCategories, py::arg("analysisType"))
    .def("attributes", &smtk::attribute::Collection::attributes, py::arg("result"))
    .def("attributes", [](const smtk::attribute::Collection& sys){ std::vector<smtk::attribute::AttributePtr> result; sys.attributes(result); return result; })
    .def("categories", &smtk::attribute::Collection::categories)
    .def("copyAttribute", &smtk::attribute::Collection::copyAttribute, py::arg("att"), py::arg("copyModelAssociations") = false, py::arg("options") = 0)
    .def("copyDefinition", &smtk::attribute::Collection::copyDefinition, py::arg("def"), py::arg("options") = 0)
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &, ::std::string const &)) &smtk::attribute::Collection::createAttribute, py::arg("name"), py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::smtk::attribute::DefinitionPtr)) &smtk::attribute::Collection::createAttribute, py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &)) &smtk::attribute::Collection::createAttribute, py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::Collection::createAttribute, py::arg("name"), py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &, ::std::string const &, ::smtk::common::UUID const &)) &smtk::attribute::Collection::createAttribute, py::arg("name"), py::arg("type"), py::arg("id"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &, ::smtk::attribute::DefinitionPtr, ::smtk::common::UUID const &)) &smtk::attribute::Collection::createAttribute, py::arg("name"), py::arg("def"), py::arg("id"))
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::Collection::*)(::std::string const &, ::std::string const &)) &smtk::attribute::Collection::createDefinition, py::arg("typeName"), py::arg("baseTypeName") = "")
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::Collection::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::Collection::createDefinition, py::arg("name"), py::arg("baseDefiniiton"))
    .def("createUniqueName", &smtk::attribute::Collection::createUniqueName, py::arg("type"))
    .def("defineAnalysis", &smtk::attribute::Collection::defineAnalysis, py::arg("analysisName"), py::arg("categories"))
    .def("definitions", &smtk::attribute::Collection::definitions, py::arg("result"))
    .def("definitions", [](const smtk::attribute::Collection& sys){ std::vector<smtk::attribute::DefinitionPtr> result; sys.definitions(result); return result; })
    .def("derivedDefinitions", &smtk::attribute::Collection::derivedDefinitions, py::arg("def"), py::arg("result"))
    .def("findAllDerivedDefinitions", &smtk::attribute::Collection::findAllDerivedDefinitions, py::arg("def"), py::arg("concreteOnly"), py::arg("result"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::std::string const &) const) &smtk::attribute::Collection::findAttribute, py::arg("name"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Collection::*)(::smtk::common::UUID const &) const) &smtk::attribute::Collection::findAttribute, py::arg("id"))
    .def("findAttributes", (void (smtk::attribute::Collection::*)(::std::string const &, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::Collection::findAttributes, py::arg("type"), py::arg("result"))
    .def("findAttributes", (std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > (smtk::attribute::Collection::*)(::std::string const &) const) &smtk::attribute::Collection::findAttributes, py::arg("type"))
    .def("findAttributes", (void (smtk::attribute::Collection::*)(::smtk::attribute::DefinitionPtr, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::Collection::findAttributes, py::arg("def"), py::arg("result"))
    .def("findBaseDefinitions", &smtk::attribute::Collection::findBaseDefinitions, py::arg("result"))
    .def("findDefinition", &smtk::attribute::Collection::findDefinition, py::arg("type"))
    .def("findDefinitionAttributes", &smtk::attribute::Collection::findDefinitionAttributes, py::arg("type"), py::arg("result"))
    .def("findDefinitions", &smtk::attribute::Collection::findDefinitions, py::arg("mask"), py::arg("result"))
    .def("findIsUniqueBaseClass", &smtk::attribute::Collection::findIsUniqueBaseClass, py::arg("attDef"))
    .def("findTopLevelView", &smtk::attribute::Collection::findTopLevelView)
    .def("findTopLevelViews", &smtk::attribute::Collection::findTopLevelViews)
    .def("findView", &smtk::attribute::Collection::findView, py::arg("title"))
    .def("findViewByType", &smtk::attribute::Collection::findViewByType, py::arg("vtype"))
    .def("hasAttributes", &smtk::attribute::Collection::hasAttributes)
    .def("numberOfAdvanceLevels", &smtk::attribute::Collection::numberOfAdvanceLevels)
    .def("numberOfAnalyses", &smtk::attribute::Collection::numberOfAnalyses)
    .def("numberOfCategories", &smtk::attribute::Collection::numberOfCategories)
    .def("refModelManager", &smtk::attribute::Collection::refModelManager)
    .def("removeAttribute", &smtk::attribute::Collection::removeAttribute, py::arg("att"))
    .def("rename", &smtk::attribute::Collection::rename, py::arg("att"), py::arg("newName"))
    .def("setAdvanceLevelColor", &smtk::attribute::Collection::setAdvanceLevelColor, py::arg("level"), py::arg("l_color"))
    .def("setRefModelManager", &smtk::attribute::Collection::setRefModelManager, py::arg("refModelMgr"))
    .def("updateCategories", &smtk::attribute::Collection::updateCategories)
    .def("updateDerivedDefinitionIndexOffsets", &smtk::attribute::Collection::updateDerivedDefinitionIndexOffsets, py::arg("def"))
    .def("views", &smtk::attribute::Collection::views)
    .def_static("New", [](){ return smtk::attribute::Collection::create(); }, py::return_value_policy::take_ownership)
    .def_static("create", [](){ return smtk::attribute::Collection::create(); }, py::return_value_policy::take_ownership)
    ;
  py::enum_<smtk::attribute::Collection::CopyOptions>(instance, "CopyOptions")
    .value("COPY_ASSOCIATIONS", smtk::attribute::Collection::CopyOptions::COPY_ASSOCIATIONS)
    .value("FORCE_COPY_ASSOCIATIONS", smtk::attribute::Collection::CopyOptions::FORCE_COPY_ASSOCIATIONS)
    .export_values();
  return instance;
}

#endif
