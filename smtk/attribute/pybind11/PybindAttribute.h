//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind___smtk_attribute_Attribute_h
#define pybind___smtk_attribute_Attribute_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/Attribute.h"

#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/io/Logger.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"
#include "smtk/simulation/UserData.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::attribute::Attribute > pybind11_init_smtk_attribute_Attribute(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Attribute, smtk::resource::Component > instance(m, "Attribute", py::dynamic_attr());
  instance
    .def(py::init<::smtk::attribute::Attribute const &>())
    .def("deepcopy", (smtk::attribute::Attribute & (smtk::attribute::Attribute::*)(::smtk::attribute::Attribute const &)) &smtk::attribute::Attribute::operator=)
    .def_static("New", (smtk::attribute::AttributePtr (*)(::std::string const &, const ::smtk::attribute::DefinitionPtr&)) &smtk::attribute::Attribute::New, py::arg("myName"), py::arg("myDefinition"))
    .def_static("New", (smtk::attribute::AttributePtr (*)(::std::string const &, const ::smtk::attribute::DefinitionPtr&, ::smtk::common::UUID const &)) &smtk::attribute::Attribute::New, py::arg("myName"), py::arg("myDefinition"), py::arg("myId"))
    .def("appliesToBoundaryNodes", &smtk::attribute::Attribute::appliesToBoundaryNodes)
    .def("appliesToInteriorNodes", &smtk::attribute::Attribute::appliesToInteriorNodes)
    .def("assign", (bool (smtk::attribute::Attribute::*)(const ::smtk::attribute::AttributePtr &, const smtk::attribute::CopyAssignmentOptions&)) &smtk::attribute::Attribute::assign, py::arg("sourceAttribute"), py::arg("options"))
    .def("assign", (bool (smtk::attribute::Attribute::*)(const ::smtk::attribute::AttributePtr &, const smtk::attribute::CopyAssignmentOptions&, smtk::io::Logger&)) &smtk::attribute::Attribute::assign, py::arg("sourceAttribute"), py::arg("options"), py::arg("logger"))
    .def("associatedObjects", (smtk::attribute::ReferenceItemPtr (smtk::attribute::Attribute::*)()) &smtk::attribute::Attribute::associatedObjects)
    .def("associate", &smtk::attribute::Attribute::associate)
    .def("associateEntity", (bool (smtk::attribute::Attribute::*)(::smtk::common::UUID const &)) &smtk::attribute::Attribute::associateEntity, py::arg("entity"))
    .def("associateEntity", (bool (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &)) &smtk::attribute::Attribute::associateEntity, py::arg("entity"))
    .def("associatedModelEntityIds", &smtk::attribute::Attribute::associatedModelEntityIds)
    .def("associations", (smtk::attribute::ConstReferenceItemPtr (smtk::attribute::Attribute::*)() const) &smtk::attribute::Attribute::associations)
    .def("associations", (smtk::attribute::ReferenceItemPtr (smtk::attribute::Attribute::*)()) &smtk::attribute::Attribute::associations)
    // NOTE that the Python form of this method is returning a copy since Python
    // doessn't support const references
    .def("categories", &smtk::attribute::Attribute::categories)
    .def("clearAllUserData", &smtk::attribute::Attribute::clearAllUserData)
    .def("clearUserData", &smtk::attribute::Attribute::clearUserData, py::arg("key"))
    .def("color", &smtk::attribute::Attribute::color)
    .def("definition", &smtk::attribute::Attribute::definition)
    .def("disassociate", (bool (smtk::attribute::Attribute::*)(::smtk::resource::PersistentObjectPtr, ::smtk::attribute::AttributePtr&, bool)) &smtk::attribute::Attribute::disassociate, py::arg("object"), py::arg("probAtt"), py::arg("reverse") = true)
     .def("disassociate", (bool (smtk::attribute::Attribute::*)(::smtk::resource::PersistentObjectPtr, bool)) &smtk::attribute::Attribute::disassociate, py::arg("object"), py::arg("reverse") = true)
    .def("disassociateEntity", (void (smtk::attribute::Attribute::*)(::smtk::common::UUID const &, bool)) &smtk::attribute::Attribute::disassociateEntity, py::arg("entity"), py::arg("reverse") = true)
    .def("disassociateEntity", (void (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &, bool)) &smtk::attribute::Attribute::disassociateEntity, py::arg("entity"), py::arg("reverse") = true)
    .def("_find", (smtk::attribute::ItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::Attribute::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("_find", (smtk::attribute::ConstItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::Attribute::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::RECURSIVE_ACTIVE)
    .def("findComponent", (smtk::attribute::ComponentItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findComponent, py::arg("name"))
    .def("findComponent", (smtk::attribute::ConstComponentItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findComponent, py::arg("name"))
    .def("findDateTime", (smtk::attribute::DateTimeItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findDateTime, py::arg("name"))
    .def("findDateTime", (smtk::attribute::ConstDateTimeItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findDateTime, py::arg("name"))
    .def("findDirectory", (smtk::attribute::DirectoryItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findDirectory, py::arg("name"))
    .def("findDirectory", (smtk::attribute::ConstDirectoryItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findDirectory, py::arg("name"))
    .def("findDouble", (smtk::attribute::DoubleItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findDouble, py::arg("name"))
    .def("findDouble", (smtk::attribute::ConstDoubleItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findDouble, py::arg("name"))
    .def("findFile", (smtk::attribute::FileItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findFile, py::arg("name"))
    .def("findFile", (smtk::attribute::ConstFileItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findFile, py::arg("name"))
    .def("findGroup", (smtk::attribute::GroupItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findGroup, py::arg("name"))
    .def("findGroup", (smtk::attribute::ConstGroupItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findGroup, py::arg("name"))
    .def("findInt", (smtk::attribute::IntItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findInt, py::arg("name"))
    .def("findInt", (smtk::attribute::ConstIntItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findInt, py::arg("name"))
    .def("findModelEntity", (smtk::attribute::ModelEntityItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findModelEntity, py::arg("name"))
    .def("findModelEntity", (smtk::attribute::ConstModelEntityItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findModelEntity, py::arg("name"))
    .def("findResource", (smtk::attribute::ResourceItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findResource, py::arg("name"))
    .def("findResource", (smtk::attribute::ConstResourceItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findResource, py::arg("name"))
    .def("findString", (smtk::attribute::StringItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findString, py::arg("name"))
    .def("findString", (smtk::attribute::ConstStringItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findString, py::arg("name"))
    .def("findVoid", (smtk::attribute::VoidItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findVoid, py::arg("name"))
    .def("findVoid", (smtk::attribute::ConstVoidItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findVoid, py::arg("name"))
    .def("id", &smtk::attribute::Attribute::id)
    .def("isA", &smtk::attribute::Attribute::isA, py::arg("def"))
    .def("isAboutToBeDeleted", &smtk::attribute::Attribute::isAboutToBeDeleted)
    .def("isColorSet", &smtk::attribute::Attribute::isColorSet)
    .def("isObjectAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::common::UUID const &) const) &smtk::attribute::Attribute::isObjectAssociated, py::arg("uid"))
    .def("isObjectAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::resource::PersistentObjectPtr const &) const) &smtk::attribute::Attribute::isObjectAssociated, py::arg("componentPtr"))
    .def("isEntityAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::common::UUID const &) const) &smtk::attribute::Attribute::isEntityAssociated, py::arg("entity"))
    .def("isEntityAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &) const) &smtk::attribute::Attribute::isEntityAssociated, py::arg("entityref"))
    .def("isRelevant", &smtk::attribute::Attribute::isRelevant, py::arg("includeCategoryCheck") = true, py::arg("includeReadAccess") = false, py::arg("readAccessLevel") = 0)
    .def("isValid", (bool (smtk::attribute::Attribute::*)(bool) const) &smtk::attribute::Attribute::isValid, py::arg("useActiveCategories") = true)
    .def("isValid", (bool (smtk::attribute::Attribute::*)(std::set<std::string> const &) const) &smtk::attribute::Attribute::isValid, py::arg("categories"))
    .def("items", &smtk::attribute::Attribute::items)
    .def("_item", &smtk::attribute::Attribute::item, py::arg("ith"))
    .def("_itemAtPath", (smtk::attribute::ItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::std::string const &, bool)) &smtk::attribute::Attribute::itemAtPath, py::arg("path"), py::arg("seps") = "/", py::arg("activeOnly") = false)
    .def("itemPath", [](const smtk::attribute::Attribute& att, const smtk::attribute::ItemPtr& item, const std::string& sep)
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Attribute::itemPath(const ItemPtr& item, const std::string& sep) const"
        << " has been deprecated.  The replacement is const std::string& sep)");
      std::string result;

      // Make sure this item actually belongs to this attribute. Otherwise the resulting
      // path will be incorrect.
      if ((item == nullptr) || (item->attribute()->id() != att.id()))
      {
        return result;
      }

      return item->path(sep);
    }, py::arg("item"), py::arg("separator") = "/")
    .def("name", &smtk::attribute::Attribute::name)
    .def("numberOfItems", &smtk::attribute::Attribute::numberOfItems)
    .def("removeAllAssociations", &smtk::attribute::Attribute::removeAllAssociations)
    .def("setAppliesToBoundaryNodes", &smtk::attribute::Attribute::setAppliesToBoundaryNodes, py::arg("appliesValue"))
    .def("setAppliesToInteriorNodes", &smtk::attribute::Attribute::setAppliesToInteriorNodes, py::arg("appliesValue"))
    .def("setColor", (void (smtk::attribute::Attribute::*)(double, double, double, double)) &smtk::attribute::Attribute::setColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setColor", (void (smtk::attribute::Attribute::*)(double const *)) &smtk::attribute::Attribute::setColor, py::arg("l_color"))
    .def("setUserData", &smtk::attribute::Attribute::setUserData, py::arg("key"), py::arg("value"))
    .def("attributeResource", &smtk::attribute::Attribute::attributeResource)
    .def("type", &smtk::attribute::Attribute::type)
    .def("types", &smtk::attribute::Attribute::types)
    .def("unsetColor", &smtk::attribute::Attribute::unsetColor)
    .def("userData", &smtk::attribute::Attribute::userData, py::arg("key"))
    .def("advanceLevel", &smtk::attribute::Attribute::advanceLevel, py::arg("mode") = 0)
    .def("localAdvanceLevel", &smtk::attribute::Attribute::localAdvanceLevel, py::arg("mode") = 0)
    .def("setlocalAdvanceLevel", &smtk::attribute::Attribute::setLocalAdvanceLevel, py::arg("mode"), py::arg("level"))
    .def("unsetLocalAdvanceLevel", &smtk::attribute::Attribute::unsetLocalAdvanceLevel, py::arg("mode") = 0)
    .def("hasLocalAdvanceLevelInfo", &smtk::attribute::Attribute::hasLocalAdvanceLevelInfo, py::arg("mode") = 0)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Component> i) {
        return std::dynamic_pointer_cast<smtk::attribute::Attribute>(i);
      })
    ;
    ;
  return instance;
}

#endif
