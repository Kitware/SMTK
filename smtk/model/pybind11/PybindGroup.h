//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Group_h
#define pybind_smtk_model_Group_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/model/Group.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace py = pybind11;

py::class_< smtk::model::Group, smtk::model::EntityRef > pybind11_init_smtk_model_Group(py::module &m)
{
  py::class_< smtk::model::Group, smtk::model::EntityRef > instance(m, "Group");
  instance
    .def(py::init<::smtk::model::Group const &>())
    .def(py::init<>())
    .def(py::init<::smtk::model::EntityRef const &>())
    .def(py::init<::smtk::model::ManagerPtr, ::smtk::common::UUID const &>())
    .def("__ne__", (bool (smtk::model::Group::*)(::smtk::model::EntityRef const &) const) &smtk::model::Group::operator!=)
    .def("deepcopy", (smtk::model::Group & (smtk::model::Group::*)(::smtk::model::Group const &)) &smtk::model::Group::operator=)
    .def("__eq__", (bool (smtk::model::Group::*)(::smtk::model::EntityRef const &) const) &smtk::model::Group::operator==)
    .def("addEntity", &smtk::model::Group::addEntity, py::arg("entity"))
    .def("classname", &smtk::model::Group::classname)
    .def("findFirstNonGroupMember", &smtk::model::Group::findFirstNonGroupMember)
    .def("isValid", (bool (smtk::model::Group::*)() const) &smtk::model::Group::isValid)
    // .def("isValid", (bool (smtk::model::Group::*)(::smtk::model::Entity * *) const) &smtk::model::Group::isValid, py::arg("entRec"))
    .def("meetsMembershipConstraints", &smtk::model::Group::meetsMembershipConstraints, py::arg("prospectiveMember"))
    .def("membershipMask", &smtk::model::Group::membershipMask)
    .def("parent", &smtk::model::Group::parent)
    .def("removeEntity", &smtk::model::Group::removeEntity, py::arg("entity"))
    .def("setMembershipMask", &smtk::model::Group::setMembershipMask, py::arg("mask"))
    .def("members", [](const smtk::model::Group& g){
        smtk::model::EntityRefArray tmp = g.members<smtk::model::EntityRefArray>();
        std::vector<smtk::model::EntityRef*> ret;
        for (smtk::model::EntityRefArray::iterator it = tmp.begin(); it != tmp.end(); ++it)
          ret.push_back(new smtk::model::EntityRef(*it));
        return ret;
      })
    ;
  return instance;
}

#endif
