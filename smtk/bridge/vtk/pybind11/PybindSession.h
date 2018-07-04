//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_vtk_Session_h
#define pybind_smtk_bridge_vtk_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/vtk/Session.h"

#include "smtk/model/Model.h"
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

void pybind11_init_smtk_bridge_vtk_EntityType(py::module &m)
{
  py::enum_<smtk::bridge::vtk::EntityType>(m, "EntityType")
    .value("EXO_MODEL", smtk::bridge::vtk::EntityType::EXO_MODEL)
    .value("EXO_BLOCK", smtk::bridge::vtk::EntityType::EXO_BLOCK)
    .value("EXO_SIDE_SET", smtk::bridge::vtk::EntityType::EXO_SIDE_SET)
    .value("EXO_NODE_SET", smtk::bridge::vtk::EntityType::EXO_NODE_SET)
    .value("EXO_BLOCKS", smtk::bridge::vtk::EntityType::EXO_BLOCKS)
    .value("EXO_SIDE_SETS", smtk::bridge::vtk::EntityType::EXO_SIDE_SETS)
    .value("EXO_NODE_SETS", smtk::bridge::vtk::EntityType::EXO_NODE_SETS)
    .value("EXO_LABEL_MAP", smtk::bridge::vtk::EntityType::EXO_LABEL_MAP)
    .value("EXO_LABEL", smtk::bridge::vtk::EntityType::EXO_LABEL)
    .value("EXO_INVALID", smtk::bridge::vtk::EntityType::EXO_INVALID)
    .export_values();
}

py::class_< smtk::bridge::vtk::EntityHandle > pybind11_init_smtk_bridge_vtk_EntityHandle(py::module &m)
{
  py::class_< smtk::bridge::vtk::EntityHandle > instance(m, "EntityHandle");
  instance
    .def(py::init<::smtk::bridge::vtk::EntityHandle const &>())
    .def(py::init<>())
    .def(py::init<int, ::vtkDataObject *, ::smtk::bridge::vtk::SessionPtr>())
    .def(py::init<int, ::vtkDataObject *, ::vtkDataObject *, int, ::smtk::bridge::vtk::SessionPtr>())
    .def("deepcopy", (smtk::bridge::vtk::EntityHandle & (smtk::bridge::vtk::EntityHandle::*)(::smtk::bridge::vtk::EntityHandle const &)) &smtk::bridge::vtk::EntityHandle::operator=)
    .def("entityType", &smtk::bridge::vtk::EntityHandle::entityType)
    .def("isValid", &smtk::bridge::vtk::EntityHandle::isValid)
    .def("modelNumber", &smtk::bridge::vtk::EntityHandle::modelNumber)
    .def("name", &smtk::bridge::vtk::EntityHandle::name)
    .def("parent", &smtk::bridge::vtk::EntityHandle::parent)
    .def("pedigree", &smtk::bridge::vtk::EntityHandle::pedigree)
    .def("visible", &smtk::bridge::vtk::EntityHandle::visible)
    .def_readwrite("m_modelNumber", &smtk::bridge::vtk::EntityHandle::m_modelNumber)
    .def_readwrite("m_object", &smtk::bridge::vtk::EntityHandle::m_object)
    .def_readwrite("m_session", &smtk::bridge::vtk::EntityHandle::m_session)
    ;
  return instance;
}

PySharedPtrClass< smtk::bridge::vtk::Session, smtk::model::Session > pybind11_init_smtk_bridge_vtk_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::vtk::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def_static("SMTK_CHILDREN", &smtk::bridge::vtk::Session::SMTK_CHILDREN)
    .def_static("SMTK_DIMENSION", &smtk::bridge::vtk::Session::SMTK_DIMENSION)
    .def_static("SMTK_GROUP_TYPE", &smtk::bridge::vtk::Session::SMTK_GROUP_TYPE)
    .def_static("SMTK_LABEL_VALUE", &smtk::bridge::vtk::Session::SMTK_LABEL_VALUE)
    .def_static("SMTK_OUTER_LABEL", &smtk::bridge::vtk::Session::SMTK_OUTER_LABEL)
    .def_static("SMTK_PEDIGREE", &smtk::bridge::vtk::Session::SMTK_PEDIGREE)
    .def_static("SMTK_UUID_KEY", &smtk::bridge::vtk::Session::SMTK_UUID_KEY)
    .def_static("SMTK_VISIBILITY", &smtk::bridge::vtk::Session::SMTK_VISIBILITY)
    .def("addModel", &smtk::bridge::vtk::Session::addModel, py::arg("model"))
    .def("allSupportedInformation", &smtk::bridge::vtk::Session::allSupportedInformation)
    .def("className", &smtk::bridge::vtk::Session::className)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Session> (*)()) &smtk::bridge::vtk::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::vtk::Session> (*)(::std::shared_ptr<smtk::bridge::vtk::Session> &)) &smtk::bridge::vtk::Session::create, py::arg("ref"))
    // .def("findOperatorConstructor", &smtk::bridge::vtk::Session::findOperatorConstructor, py::arg("opName"))
    .def("name", &smtk::bridge::vtk::Session::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::vtk::Session> (smtk::bridge::vtk::Session::*)() const) &smtk::bridge::vtk::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::vtk::Session> (smtk::bridge::vtk::Session::*)()) &smtk::bridge::vtk::Session::shared_from_this)
    .def_static("staticClassName", &smtk::bridge::vtk::Session::staticClassName)
    .def("toEntity", &smtk::bridge::vtk::Session::toEntity, py::arg("eid"))
    .def("toEntityRef", &smtk::bridge::vtk::Session::toEntityRef, py::arg("ent"))
    ;
  return instance;
}

void pybind11_init_smtk_bridge_vtk_EntityTypeNameString(py::module &m)
{
  m.def("EntityTypeNameString", &smtk::bridge::vtk::EntityTypeNameString, "", py::arg("etype"));
}

#endif
