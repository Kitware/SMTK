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
#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/Resource.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/io/Logger.h"
#include "smtk/view/Configuration.h"
#include "smtk/model/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Resource> pybind11_init_smtk_attribute_Resource(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Resource, smtk::resource::Resource> instance(m, "Resource");
  instance
    .def("activeCategories", &smtk::attribute::Resource::activeCategories)
    .def("activeCategoriesEnabled", &smtk::attribute::Resource::activeCategoriesEnabled)
    .def("addAdvanceLevel", &smtk::attribute::Resource::addAdvanceLevel, py::arg("level"), py::arg("label"), py::arg("l_color") = 0)
    .def("addUniqueRole", &smtk::attribute::Resource::addUniqueRoles, py::arg("role"))
    .def("addUniqueRoles", &smtk::attribute::Resource::addUniqueRoles, py::arg("roles"))
    .def("addView", &smtk::attribute::Resource::addView, py::arg("arg0"))
    .def("addStyle", &smtk::attribute::Resource::addStyle, py::arg("defTypeName"), py::arg("style"))
    .def("advanceLevelColor", &smtk::attribute::Resource::advanceLevelColor, py::arg("level"))
    .def("advanceLevels", &smtk::attribute::Resource::advanceLevels)
    .def("analyses", &smtk::attribute::Resource::analyses)
    .def("associations", &smtk::attribute::Resource::associations)
    .def("associate", &smtk::attribute::Resource::associate)
    .def("attributes", (std::set<smtk::attribute::AttributePtr> (smtk::attribute::Resource::*)(const smtk::resource::ConstPersistentObjectPtr&) const) &smtk::attribute::Resource::attributes, py::arg("object"))
    .def("attributes", [](const smtk::attribute::Resource& sys){ std::vector<smtk::attribute::AttributePtr> result; sys.attributes(result); return result; })
    .def("categories", &smtk::attribute::Resource::categories)
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
    .def("defaultNameSeparator", &smtk::attribute::Resource::defaultNameSeparator)
    .def("definitions", [](const smtk::attribute::Resource& sys){ std::vector<smtk::attribute::DefinitionPtr> result; sys.definitions(result); return result; })
    .def("derivedDefinitions", [](const smtk::attribute::Resource& res, smtk::attribute::DefinitionPtr defn) { std::vector<smtk::attribute::DefinitionPtr> result; res.derivedDefinitions(defn, result); return result; })
    .def("disassociate", &smtk::attribute::Resource::disassociate)
    .def("findAllDerivedDefinitions", [](const smtk::attribute::Resource& res, smtk::attribute::DefinitionPtr defn, bool concreteOnly) { std::vector<smtk::attribute::DefinitionPtr> result; res.findAllDerivedDefinitions(defn, concreteOnly, result); return result; })
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::std::string const &) const) &smtk::attribute::Resource::findAttribute, py::arg("name"))
    .def("findAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(::smtk::common::UUID const &) const) &smtk::attribute::Resource::findAttribute, py::arg("id"))
    .def("findAttributes", (std::vector<std::shared_ptr<smtk::attribute::Attribute>, std::allocator<std::shared_ptr<smtk::attribute::Attribute> > > (smtk::attribute::Resource::*)(::std::string const &) const) &smtk::attribute::Resource::findAttributes, py::arg("type"))
    .def("findAttributes", [](const smtk::attribute::Resource& res, smtk::attribute::DefinitionPtr defn) { std::vector<smtk::attribute::AttributePtr> result; res.findAttributes(defn, result); return result; })
    .def("findBaseDefinitions", [](const smtk::attribute::Resource& res) { std::vector<smtk::attribute::DefinitionPtr> result; res.findBaseDefinitions(result); return result; })
    .def("findDefinition", &smtk::attribute::Resource::findDefinition, py::arg("type"))
    .def("findDefinitionAttributes", [](const smtk::attribute::Resource& res, const std::string& type) { std::vector<smtk::attribute::AttributePtr> result; res.findDefinitionAttributes(type, result); return result; })
    .def("findDefinitions", [](const smtk::attribute::Resource& res, unsigned long mask) { std::vector<smtk::attribute::DefinitionPtr> result; res.findDefinitions(mask, result); return result; })
    .def("findIsUniqueBaseClass", &smtk::attribute::Resource::findIsUniqueBaseClass, py::arg("attDef"))
    .def("findTopLevelView", &smtk::attribute::Resource::findTopLevelView)
    .def("findTopLevelViews", &smtk::attribute::Resource::findTopLevelViews)
    .def("findView", &smtk::attribute::Resource::findView, py::arg("title"))
    .def("findViewByType", &smtk::attribute::Resource::findViewByType, py::arg("vtype"))
    .def("findStyle", &smtk::attribute::Resource::findStyle, py::arg("stype"), py::arg("styleName"))
    .def("findStyles", &smtk::attribute::Resource::findStyles, py::arg("stype"))
    .def("hasAttributes", (bool (smtk::attribute::Resource::*) () const) &smtk::attribute::Resource::hasAttributes)
    .def("hasAttributes", (bool (smtk::attribute::Resource::*) (const smtk::resource::ConstPersistentObjectPtr&) const) &smtk::attribute::Resource::hasAttributes, py::arg("object"))
    .def("isRoleUnique", &smtk::attribute::Resource::isRoleUnique)
    .def("numberOfAdvanceLevels", &smtk::attribute::Resource::numberOfAdvanceLevels)
    .def("numberOfCategories", &smtk::attribute::Resource::numberOfCategories)
    .def("passActiveCategoryCheck", (bool (smtk::attribute::Resource::*) (const smtk::attribute::Categories::Set& cats) const) &smtk::attribute::Resource::passActiveCategoryCheck, py::arg("categorySet"))
    .def("passActiveCategoryCheck", (bool (smtk::attribute::Resource::*) (const smtk::attribute::Categories& cats) const) &smtk::attribute::Resource::passActiveCategoryCheck, py::arg("categories"))
    .def("removeAttribute", &smtk::attribute::Resource::removeAttribute, py::arg("att"))
    .def("rename", &smtk::attribute::Resource::rename, py::arg("att"), py::arg("newName"))
    .def("resetDefaultNameSeparator", &smtk::attribute::Resource::resetDefaultNameSeparator)
    .def("setActiveCategories", &smtk::attribute::Resource::setActiveCategories, py::arg("categories"))
    .def("setActiveCategoriesEnabled", &smtk::attribute::Resource::setActiveCategoriesEnabled, py::arg("mode"))
    .def("setAdvanceLevelColor", &smtk::attribute::Resource::setAdvanceLevelColor, py::arg("level"), py::arg("l_color"))
    .def("setDefaultNameSeparator", &smtk::attribute::Resource::setDefaultNameSeparator, py::arg("separator"))
    .def("uniqueRoles", &smtk::attribute::Resource::uniqueRoles)
    .def("finalizeDefinitions", &smtk::attribute::Resource::finalizeDefinitions)
    .def("updateDerivedDefinitionIndexOffsets", &smtk::attribute::Resource::updateDerivedDefinitionIndexOffsets, py::arg("def"))
    .def("views", &smtk::attribute::Resource::views)
    .def("styles", &smtk::attribute::Resource::styles)
    .def_static("createAttributeQuery",  [](const smtk::attribute::DefinitionPtr& def){ return smtk::attribute::Resource::createAttributeQuery(def); }, py::arg("def"))
    .def_static("createAttributeQuery",  [](const std::string& str){ return smtk::attribute::Resource::createAttributeQuery(str); }, py::arg("def"))
    .def_static("New", [](){ return smtk::attribute::Resource::create(); }, py::return_value_policy::take_ownership)
    .def_static("create", [](){ return smtk::attribute::Resource::create(); }, py::return_value_policy::take_ownership)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::attribute::Resource>(i);
      })
    .def("copyAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(const smtk::attribute::AttributePtr& att, const smtk::attribute::CopyAssignmentOptions&)) &smtk::attribute::Resource::copyAttribute, py::arg("sourceAttribute"), py::arg("options") = smtk::attribute::CopyAssignmentOptions())
    .def("copyAttribute", (smtk::attribute::AttributePtr (smtk::attribute::Resource::*)(const smtk::attribute::AttributePtr& att, const smtk::attribute::CopyAssignmentOptions&, smtk::io::Logger&)) &smtk::attribute::Resource::copyAttribute, py::arg("sourceAttribute"), py::arg("options"), py::arg("logger"))
    .def("copyAttribute", [](smtk::attribute::Resource& res, smtk::attribute::AttributePtr att, bool copyAssoc, unsigned int options)
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Resource::copyAttribute(smtk::attribute::AttributePtr, bool, unsigned int)"
        << " has been deprecated.  The replacement is Resource::copyAttribute(const smtk::attribute::AttributePtr&, const AttributeCopyOptions&,"
        << " const AttributeAssignmentOptions&, const ItemAssignmentOptions& itemOptions)");
      smtk::attribute::CopyAssignmentOptions opts;
      smtk::attribute::Item::mapOldAssignmentOptions(opts, options);
      opts.copyOptions.setCopyDefinition(true);

      if (copyAssoc)
      {
        opts.attributeOptions.setCopyAssociations(true);
      }

      return res.copyAttribute(att, opts);
    }, py::arg("attribute"), py::arg("copyAssoc"), py::arg("options"))
    ;
  return instance;
}

#endif
