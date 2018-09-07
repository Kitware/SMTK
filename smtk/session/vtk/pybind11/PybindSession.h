//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Session_h
#define pybind_smtk_session_vtk_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/Session.h"

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

void pybind11_init_smtk_session_vtk_EntityType(py::module &m)
{
  py::enum_<smtk::session::vtk::EntityType>(m, "EntityType")
    .value("EXO_MODEL", smtk::session::vtk::EntityType::EXO_MODEL)
    .value("EXO_BLOCK", smtk::session::vtk::EntityType::EXO_BLOCK)
    .value("EXO_SIDE_SET", smtk::session::vtk::EntityType::EXO_SIDE_SET)
    .value("EXO_NODE_SET", smtk::session::vtk::EntityType::EXO_NODE_SET)
    .value("EXO_BLOCKS", smtk::session::vtk::EntityType::EXO_BLOCKS)
    .value("EXO_SIDE_SETS", smtk::session::vtk::EntityType::EXO_SIDE_SETS)
    .value("EXO_NODE_SETS", smtk::session::vtk::EntityType::EXO_NODE_SETS)
    .value("EXO_LABEL_MAP", smtk::session::vtk::EntityType::EXO_LABEL_MAP)
    .value("EXO_LABEL", smtk::session::vtk::EntityType::EXO_LABEL)
    .value("EXO_INVALID", smtk::session::vtk::EntityType::EXO_INVALID)
    .export_values();
}

py::class_< smtk::session::vtk::EntityHandle > pybind11_init_smtk_session_vtk_EntityHandle(py::module &m)
{
  py::class_< smtk::session::vtk::EntityHandle > instance(m, "EntityHandle");
  instance
    .def(py::init<::smtk::session::vtk::EntityHandle const &>())
    .def(py::init<>())
    .def(py::init<int, ::vtkDataObject *, ::smtk::session::vtk::SessionPtr>())
    .def(py::init<int, ::vtkDataObject *, ::vtkDataObject *, int, ::smtk::session::vtk::SessionPtr>())
    .def("deepcopy", (smtk::session::vtk::EntityHandle & (smtk::session::vtk::EntityHandle::*)(::smtk::session::vtk::EntityHandle const &)) &smtk::session::vtk::EntityHandle::operator=)
    .def("entityType", &smtk::session::vtk::EntityHandle::entityType)
    .def("isValid", &smtk::session::vtk::EntityHandle::isValid)
    .def("modelNumber", &smtk::session::vtk::EntityHandle::modelNumber)
    .def("name", &smtk::session::vtk::EntityHandle::name)
    .def("parent", &smtk::session::vtk::EntityHandle::parent)
    .def("pedigree", &smtk::session::vtk::EntityHandle::pedigree)
    .def("visible", &smtk::session::vtk::EntityHandle::visible)
    .def_readwrite("m_modelNumber", &smtk::session::vtk::EntityHandle::m_modelNumber)
    .def_readwrite("m_object", &smtk::session::vtk::EntityHandle::m_object)
    .def_readwrite("m_session", &smtk::session::vtk::EntityHandle::m_session)
    ;
  return instance;
}

PySharedPtrClass< smtk::session::vtk::Session, smtk::model::Session > pybind11_init_smtk_session_vtk_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::vtk::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def_static("SMTK_CHILDREN", &smtk::session::vtk::Session::SMTK_CHILDREN)
    .def_static("SMTK_DIMENSION", &smtk::session::vtk::Session::SMTK_DIMENSION)
    .def_static("SMTK_GROUP_TYPE", &smtk::session::vtk::Session::SMTK_GROUP_TYPE)
    .def_static("SMTK_LABEL_VALUE", &smtk::session::vtk::Session::SMTK_LABEL_VALUE)
    .def_static("SMTK_OUTER_LABEL", &smtk::session::vtk::Session::SMTK_OUTER_LABEL)
    .def_static("SMTK_PEDIGREE", &smtk::session::vtk::Session::SMTK_PEDIGREE)
    .def_static("SMTK_UUID_KEY", &smtk::session::vtk::Session::SMTK_UUID_KEY)
    .def_static("SMTK_VISIBILITY", &smtk::session::vtk::Session::SMTK_VISIBILITY)
    .def("addModel", &smtk::session::vtk::Session::addModel, py::arg("model"))
    .def("allSupportedInformation", &smtk::session::vtk::Session::allSupportedInformation)
    .def("className", &smtk::session::vtk::Session::className)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Session> (*)()) &smtk::session::vtk::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Session> (*)(::std::shared_ptr<smtk::session::vtk::Session> &)) &smtk::session::vtk::Session::create, py::arg("ref"))
    // .def("findOperatorConstructor", &smtk::session::vtk::Session::findOperatorConstructor, py::arg("opName"))
    .def("name", &smtk::session::vtk::Session::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::vtk::Session> (smtk::session::vtk::Session::*)() const) &smtk::session::vtk::Session::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::vtk::Session> (smtk::session::vtk::Session::*)()) &smtk::session::vtk::Session::shared_from_this)
    .def_static("staticClassName", &smtk::session::vtk::Session::staticClassName)
    .def("toEntity", &smtk::session::vtk::Session::toEntity, py::arg("eid"))
    .def("toEntityRef", &smtk::session::vtk::Session::toEntityRef, py::arg("ent"))
    ;
  return instance;
}

void pybind11_init_smtk_session_vtk_EntityTypeNameString(py::module &m)
{
  m.def("EntityTypeNameString", &smtk::session::vtk::EntityTypeNameString, "", py::arg("etype"));
}

#endif
