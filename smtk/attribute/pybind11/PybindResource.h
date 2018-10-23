//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Resource_h
#define pybind_smtk_attribute_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/view/View.h"
#include "smtk/model/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::Resource, smtk::resource::Resource > pybind11_init_smtk_attribute_Resource(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Resource, smtk::resource::Resource > instance(m, "Resource");
  instance
    .def("addAdvanceLevel", &smtk::attribute::Resource::addAdvanceLevel, py::arg("level"), py::arg("label"), py::arg("l_color") = 0)
    .def("addView", &smtk::attribute::Resource::addView, py::arg("arg0"))
    .def("advanceLevelColor", &smtk::attribute::Resource::advanceLevelColor, py::arg("level"))
    .def("advanceLevels", &smtk::attribute::Resource::advanceLevels)
    .def("analyses", &smtk::attribute::Resource::analyses)
    .def("analysisCategories", &smtk::attribute::Resource::analysisCategories, py::arg("analysisType"))
    .def("associations", &smtk::attribute::Resource::associations)
    .def("associate", &smtk::attribute::Resource::associate)
    .def("attributes", (std::set<smtk::attribute::AttributePtr> (smtk::attribute::Resource::*)(const smtk::resource::ConstPersistentObjectPtr&) const) &smtk::attribute::Resource::attributes, py::arg("object"))
    .def("attributes", (void (smtk::attribute::Resource::*)(std::vector<smtk::attribute::AttributePtr>&) const) &smtk::attribute::Resource::attributes, py::arg("result"))
    .def("attributes", [](const smtk::attribute::Resource& sys){ std::vector<smtk::attribute::AttributePtr> result; sys.attributes(result); return result; })
    .def("categories", &smtk::attribute::Resource::categories)
    .def("copyAttribute", &smtk::attribute::Resource::copyAttribute, py::arg("att"), py::arg("copyModelAssociations") = false, py::arg("options") = 0)
    .def("copyDefinition", &smtk::attribute::Resource::copyDefinition, py::arg("def"), py::arg("options") = 0)
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &, ::std::string const &)) &smtk::attribute::Resource::createAttribute, py::arg("name"), py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::smtk::attribute::DefinitionPtr)) &smtk::attribute::Resource::createAttribute, py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &)) &smtk::attribute::Resource::createAttribute, py::arg("type"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::Resource::createAttribute, py::arg("name"), py::arg("def"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &, ::std::string const &, ::smtk::common::UUID const &)) &smtk::attribute::Resource::createAttribute, py::arg("name"), py::arg("type"), py::arg("id"))
    .def("createAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &, ::smtk::attribute::DefinitionPtr, ::smtk::common::UUID const &)) &smtk::attribute::Resource::createAttribute, py::arg("name"), py::arg("def"), py::arg("id"))
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::Resource::*)(::std::string const &, ::std::string const &)) &smtk::attribute::Resource::createDefinition, py::arg("typeName"), py::arg("baseTypeName") = "")
    .def("createDefinition", (smtk::attribute::DefinitionPtr (smtk::attribute::Resource::*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::Resource::createDefinition, py::arg("name"), py::arg("baseDefiniiton"))
    .def("createUniqueName", &smtk::attribute::Resource::createUniqueName, py::arg("type"))
    .def("defineAnalysis", &smtk::attribute::Resource::defineAnalysis, py::arg("analysisName"), py::arg("categories"))
    .def("definitions", &smtk::attribute::Resource::definitions, py::arg("result"))
    .def("definitions", [](const smtk::attribute::Resource& sys){ std::vector<smtk::attribute::DefinitionPtr> result; sys.definitions(result); return result; })
    .def("derivedDefinitions", &smtk::attribute::Resource::derivedDefinitions, py::arg("def"), py::arg("result"))
    .def("disassociate", &smtk::attribute::Resource::disassociate)
    .def("findAllDerivedDefinitions", &smtk::attribute::Resource::findAllDerivedDefinitions, py::arg("def"), py::arg("concreteOnly"), py::arg("result"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &) const) &smtk::attribute::Resource::findAttribute, py::arg("name"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::smtk::common::UUID const &) const) &smtk::attribute::Resource::findAttribute, py::arg("id"))
    .def("findAttributes", (void (smtk::attribute::Resource::*)(::std::string const &, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::Resource::findAttributes, py::arg("type"), py::arg("result"))
    .def("findAttributes", (std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > (smtk::attribute::Resource::*)(::std::string const &) const) &smtk::attribute::Resource::findAttributes, py::arg("type"))
    .def("findAttributes", (void (smtk::attribute::Resource::*)(::smtk::attribute::DefinitionPtr, ::std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > &) const) &smtk::attribute::Resource::findAttributes, py::arg("def"), py::arg("result"))
    .def("findBaseDefinitions", &smtk::attribute::Resource::findBaseDefinitions, py::arg("result"))
    .def("findDefinition", &smtk::attribute::Resource::findDefinition, py::arg("type"))
    .def("findDefinitionAttributes", &smtk::attribute::Resource::findDefinitionAttributes, py::arg("type"), py::arg("result"))
    .def("findDefinitions", &smtk::attribute::Resource::findDefinitions, py::arg("mask"), py::arg("result"))
    .def("findIsUniqueBaseClass", &smtk::attribute::Resource::findIsUniqueBaseClass, py::arg("attDef"))
    .def("findTopLevelView", &smtk::attribute::Resource::findTopLevelView)
    .def("findTopLevelViews", &smtk::attribute::Resource::findTopLevelViews)
    .def("findView", &smtk::attribute::Resource::findView, py::arg("title"))
    .def("findViewByType", &smtk::attribute::Resource::findViewByType, py::arg("vtype"))
    .def("hasAttributes", &smtk::attribute::Resource::hasAttributes)
    .def("numberOfAdvanceLevels", &smtk::attribute::Resource::numberOfAdvanceLevels)
    .def("numberOfAnalyses", &smtk::attribute::Resource::numberOfAnalyses)
    .def("numberOfCategories", &smtk::attribute::Resource::numberOfCategories)
    .def("removeAttribute", &smtk::attribute::Resource::removeAttribute, py::arg("att"))
    .def("rename", &smtk::attribute::Resource::rename, py::arg("att"), py::arg("newName"))
    .def("setAdvanceLevelColor", &smtk::attribute::Resource::setAdvanceLevelColor, py::arg("level"), py::arg("l_color"))
    .def("updateCategories", &smtk::attribute::Resource::updateCategories)
    .def("updateDerivedDefinitionIndexOffsets", &smtk::attribute::Resource::updateDerivedDefinitionIndexOffsets, py::arg("def"))
    .def("views", &smtk::attribute::Resource::views)
    .def_static("New", [](){ return smtk::attribute::Resource::create(); }, py::return_value_policy::take_ownership)
    .def_static("create", [](){ return smtk::attribute::Resource::create(); }, py::return_value_policy::take_ownership)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::attribute::Resource>(i);
      })
    ;
  py::enum_<smtk::attribute::Resource::CopyOptions>(instance, "CopyOptions")
    .value("COPY_ASSOCIATIONS", smtk::attribute::Resource::CopyOptions::COPY_ASSOCIATIONS)
    .value("FORCE_COPY_ASSOCIATIONS", smtk::attribute::Resource::CopyOptions::FORCE_COPY_ASSOCIATIONS)
    .export_values();
  return instance;
}

#endif
