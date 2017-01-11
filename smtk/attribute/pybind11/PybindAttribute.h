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

#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/simulation/UserData.h"

namespace py = pybind11;

PySharedPtrClass< smtk::attribute::Attribute > pybind11_init_smtk_attribute_Attribute(py::module &m)
{
  PySharedPtrClass< smtk::attribute::Attribute > instance(m, "Attribute", py::metaclass());
  instance
    .def(py::init<::smtk::attribute::Attribute const &>())
    .def("deepcopy", (smtk::attribute::Attribute & (smtk::attribute::Attribute::*)(::smtk::attribute::Attribute const &)) &smtk::attribute::Attribute::operator=)
    .def_static("New", (smtk::attribute::AttributePtr (*)(::std::string const &, ::smtk::attribute::DefinitionPtr)) &smtk::attribute::Attribute::New, py::arg("myName"), py::arg("myDefinition"))
    .def_static("New", (smtk::attribute::AttributePtr (*)(::std::string const &, ::smtk::attribute::DefinitionPtr, ::smtk::common::UUID const &)) &smtk::attribute::Attribute::New, py::arg("myName"), py::arg("myDefinition"), py::arg("myId"))
    .def("appliesToBoundaryNodes", &smtk::attribute::Attribute::appliesToBoundaryNodes)
    .def("appliesToInteriorNodes", &smtk::attribute::Attribute::appliesToInteriorNodes)
    .def("associateEntity", (bool (smtk::attribute::Attribute::*)(::smtk::common::UUID const &)) &smtk::attribute::Attribute::associateEntity, py::arg("entity"))
    .def("associateEntity", (bool (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &)) &smtk::attribute::Attribute::associateEntity, py::arg("entity"))
    .def("associatedModelEntityIds", &smtk::attribute::Attribute::associatedModelEntityIds)
    .def("associations", (smtk::attribute::ConstModelEntityItemPtr (smtk::attribute::Attribute::*)() const) &smtk::attribute::Attribute::associations)
    .def("associations", (smtk::attribute::ModelEntityItemPtr (smtk::attribute::Attribute::*)()) &smtk::attribute::Attribute::associations)
    .def("clearAllUserData", &smtk::attribute::Attribute::clearAllUserData)
    .def("clearUserData", &smtk::attribute::Attribute::clearUserData, py::arg("key"))
    .def("color", &smtk::attribute::Attribute::color)
    .def("definition", &smtk::attribute::Attribute::definition)
    .def("disassociateEntity", (void (smtk::attribute::Attribute::*)(::smtk::common::UUID const &, bool)) &smtk::attribute::Attribute::disassociateEntity, py::arg("entity"), py::arg("reverse") = true)
    .def("disassociateEntity", (void (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &, bool)) &smtk::attribute::Attribute::disassociateEntity, py::arg("entity"), py::arg("reverse") = true)
    .def("find", (smtk::attribute::ItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::smtk::attribute::SearchStyle)) &smtk::attribute::Attribute::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::ACTIVE_CHILDREN)
    .def("find", (smtk::attribute::ConstItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::smtk::attribute::SearchStyle) const) &smtk::attribute::Attribute::find, py::arg("name"), py::arg("style") = ::smtk::attribute::SearchStyle::ACTIVE_CHILDREN)
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
    .def("findMesh", (smtk::attribute::MeshItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findMesh, py::arg("name"))
    .def("findMesh", (smtk::attribute::ConstMeshItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findMesh, py::arg("name"))
    .def("findMeshSelection", (smtk::attribute::MeshSelectionItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findMeshSelection, py::arg("name"))
    .def("findMeshSelection", (smtk::attribute::ConstMeshSelectionItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findMeshSelection, py::arg("name"))
    .def("findModelEntity", (smtk::attribute::ModelEntityItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findModelEntity, py::arg("name"))
    .def("findModelEntity", (smtk::attribute::ConstModelEntityItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findModelEntity, py::arg("name"))
    .def("findRef", (smtk::attribute::RefItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findRef, py::arg("name"))
    .def("findRef", (smtk::attribute::ConstRefItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findRef, py::arg("name"))
    .def("findString", (smtk::attribute::StringItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findString, py::arg("name"))
    .def("findString", (smtk::attribute::ConstStringItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findString, py::arg("name"))
    .def("findVoid", (smtk::attribute::VoidItemPtr (smtk::attribute::Attribute::*)(::std::string const &)) &smtk::attribute::Attribute::findVoid, py::arg("name"))
    .def("findVoid", (smtk::attribute::ConstVoidItemPtr (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::findVoid, py::arg("name"))
    .def("id", &smtk::attribute::Attribute::id)
    .def("isA", &smtk::attribute::Attribute::isA, py::arg("def"))
    .def("isAboutToBeDeleted", &smtk::attribute::Attribute::isAboutToBeDeleted)
    .def("isColorSet", &smtk::attribute::Attribute::isColorSet)
    .def("isEntityAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::common::UUID const &) const) &smtk::attribute::Attribute::isEntityAssociated, py::arg("entity"))
    .def("isEntityAssociated", (bool (smtk::attribute::Attribute::*)(::smtk::model::EntityRef const &) const) &smtk::attribute::Attribute::isEntityAssociated, py::arg("entityref"))
    .def("isMemberOf", (bool (smtk::attribute::Attribute::*)(::std::string const &) const) &smtk::attribute::Attribute::isMemberOf, py::arg("category"))
    .def("isMemberOf", (bool (smtk::attribute::Attribute::*)(::std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > const &) const) &smtk::attribute::Attribute::isMemberOf, py::arg("categories"))
    .def("isValid", &smtk::attribute::Attribute::isValid)
    .def("item", &smtk::attribute::Attribute::item, py::arg("ith"))
    .def("itemAtPath", (smtk::attribute::ConstItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::std::string const &) const) &smtk::attribute::Attribute::itemAtPath, py::arg("path"), py::arg("seps") = "/")
    .def("itemAtPath", (smtk::attribute::ItemPtr (smtk::attribute::Attribute::*)(::std::string const &, ::std::string const &)) &smtk::attribute::Attribute::itemAtPath, py::arg("path"), py::arg("seps") = "/")
    .def("modelManager", &smtk::attribute::Attribute::modelManager)
    .def("name", &smtk::attribute::Attribute::name)
    .def("numberOfItems", &smtk::attribute::Attribute::numberOfItems)
    // .def("references", &smtk::attribute::Attribute::references, py::arg("list"))
    .def("references", [](const smtk::attribute::Attribute &a) { std::vector<smtk::attribute::ItemPtr> v; a.references(v); return v; })
    .def("removeAllAssociations", &smtk::attribute::Attribute::removeAllAssociations)
    .def("setAppliesToBoundaryNodes", &smtk::attribute::Attribute::setAppliesToBoundaryNodes, py::arg("appliesValue"))
    .def("setAppliesToInteriorNodes", &smtk::attribute::Attribute::setAppliesToInteriorNodes, py::arg("appliesValue"))
    .def("setColor", (void (smtk::attribute::Attribute::*)(double, double, double, double)) &smtk::attribute::Attribute::setColor, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("alpha"))
    .def("setColor", (void (smtk::attribute::Attribute::*)(double const *)) &smtk::attribute::Attribute::setColor, py::arg("l_color"))
    .def("setUserData", &smtk::attribute::Attribute::setUserData, py::arg("key"), py::arg("value"))
    .def("system", &smtk::attribute::Attribute::system)
    .def("type", &smtk::attribute::Attribute::type)
    .def("types", &smtk::attribute::Attribute::types)
    .def("unsetColor", &smtk::attribute::Attribute::unsetColor)
    .def("userData", &smtk::attribute::Attribute::userData, py::arg("key"))
    ;
  return instance;
}

#endif
