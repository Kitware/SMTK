//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_exodus_Session_h
#define pybind_smtk_bridge_exodus_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/exodus/Session.h"

#include "smtk/model/Session.h"

#include "vtkCellArray.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationObjectBaseVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

namespace py = pybind11;

void pybind11_init_smtk_bridge_exodus_EntityType(py::module &m)
{
  py::enum_<smtk::bridge::exodus::EntityType>(m, "EntityType")
    .value("EXO_MODEL", smtk::bridge::exodus::EntityType::EXO_MODEL)
    .value("EXO_BLOCK", smtk::bridge::exodus::EntityType::EXO_BLOCK)
    .value("EXO_SIDE_SET", smtk::bridge::exodus::EntityType::EXO_SIDE_SET)
    .value("EXO_NODE_SET", smtk::bridge::exodus::EntityType::EXO_NODE_SET)
    .value("EXO_BLOCKS", smtk::bridge::exodus::EntityType::EXO_BLOCKS)
    .value("EXO_SIDE_SETS", smtk::bridge::exodus::EntityType::EXO_SIDE_SETS)
    .value("EXO_NODE_SETS", smtk::bridge::exodus::EntityType::EXO_NODE_SETS)
    .value("EXO_LABEL_MAP", smtk::bridge::exodus::EntityType::EXO_LABEL_MAP)
    .value("EXO_LABEL", smtk::bridge::exodus::EntityType::EXO_LABEL)
    .value("EXO_INVALID", smtk::bridge::exodus::EntityType::EXO_INVALID)
    .export_values();
}

py::class_< smtk::bridge::exodus::EntityHandle > pybind11_init_smtk_bridge_exodus_EntityHandle(py::module &m)
{
  py::class_< smtk::bridge::exodus::EntityHandle > instance(m, "EntityHandle");
  instance
    .def(py::init<::smtk::bridge::exodus::EntityHandle const &>())
    .def(py::init<>())
    .def(py::init<int, ::vtkDataObject *, ::smtk::bridge::exodus::Session *>())
    .def(py::init<int, ::vtkDataObject *, ::vtkDataObject *, int, ::smtk::bridge::exodus::Session *>())
    .def("deepcopy", (smtk::bridge::exodus::EntityHandle & (smtk::bridge::exodus::EntityHandle::*)(::smtk::bridge::exodus::EntityHandle const &)) &smtk::bridge::exodus::EntityHandle::operator=)
    .def("entityType", &smtk::bridge::exodus::EntityHandle::entityType)
    .def("isValid", &smtk::bridge::exodus::EntityHandle::isValid)
    .def("modelNumber", &smtk::bridge::exodus::EntityHandle::modelNumber)
    .def("name", &smtk::bridge::exodus::EntityHandle::name)
    .def("parent", &smtk::bridge::exodus::EntityHandle::parent)
    .def("pedigree", &smtk::bridge::exodus::EntityHandle::pedigree)
    .def("visible", &smtk::bridge::exodus::EntityHandle::visible)
    .def_readwrite("m_modelNumber", &smtk::bridge::exodus::EntityHandle::m_modelNumber)
    .def_readwrite("m_object", &smtk::bridge::exodus::EntityHandle::m_object)
    .def_readwrite("m_session", &smtk::bridge::exodus::EntityHandle::m_session)
    ;
  return instance;
}

PySharedPtrClass< smtk::bridge::exodus::Session, smtk::model::Session > pybind11_init_smtk_bridge_exodus_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::exodus::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def_static("SMTK_CHILDREN", &smtk::bridge::exodus::Session::SMTK_CHILDREN)
    .def_static("SMTK_DIMENSION", &smtk::bridge::exodus::Session::SMTK_DIMENSION)
    .def_static("SMTK_GROUP_TYPE", &smtk::bridge::exodus::Session::SMTK_GROUP_TYPE)
    .def_static("SMTK_LABEL_VALUE", &smtk::bridge::exodus::Session::SMTK_LABEL_VALUE)
    .def_static("SMTK_OUTER_LABEL", &smtk::bridge::exodus::Session::SMTK_OUTER_LABEL)
    .def_static("SMTK_PEDIGREE", &smtk::bridge::exodus::Session::SMTK_PEDIGREE)
    .def_static("SMTK_UUID_KEY", &smtk::bridge::exodus::Session::SMTK_UUID_KEY)
    .def_static("SMTK_VISIBILITY", &smtk::bridge::exodus::Session::SMTK_VISIBILITY)
    .def("addModel", &smtk::bridge::exodus::Session::addModel, py::arg("model"))
    .def("allSupportedInformation", &smtk::bridge::exodus::Session::allSupportedInformation)
    .def("className", &smtk::bridge::exodus::Session::className)
    .def("classname", &smtk::bridge::exodus::Session::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::Session> (*)()) &smtk::bridge::exodus::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::Session> (*)(::std::shared_ptr<smtk::bridge::exodus::Session> &)) &smtk::bridge::exodus::Session::create, py::arg("ref"))
    // .def("findOperatorConstructor", &smtk::bridge::exodus::Session::findOperatorConstructor, py::arg("opName"))
    .def("findOperatorXML", &smtk::bridge::exodus::Session::findOperatorXML, py::arg("opName"))
    .def("inheritsOperators", &smtk::bridge::exodus::Session::inheritsOperators)
    .def("name", &smtk::bridge::exodus::Session::name)
    .def("registerOperator", &smtk::bridge::exodus::Session::registerOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def_static("registerStaticOperator", &smtk::bridge::exodus::Session::registerStaticOperator, py::arg("opName"), py::arg("opDescrXML"), py::arg("opCtor"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::exodus::Session> (smtk::bridge::exodus::Session::*)() const) &smtk::bridge::exodus::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::exodus::Session> (smtk::bridge::exodus::Session::*)()) &smtk::bridge::exodus::Session::shared_from_this)
    .def_static("staticClassName", &smtk::bridge::exodus::Session::staticClassName)
    .def("toEntity", &smtk::bridge::exodus::Session::toEntity, py::arg("eid"))
    .def("toEntityRef", &smtk::bridge::exodus::Session::toEntityRef, py::arg("ent"))
    ;
  return instance;
}

void pybind11_init_smtk_bridge_exodus_EntityTypeNameString(py::module &m)
{
  m.def("EntityTypeNameString", &smtk::bridge::exodus::EntityTypeNameString, "", py::arg("etype"));
}

#endif
