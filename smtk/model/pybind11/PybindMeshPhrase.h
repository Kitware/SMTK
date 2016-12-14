//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_MeshPhrase_h
#define pybind_smtk_model_MeshPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/MeshPhrase.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/DescriptivePhrase.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::MeshPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_MeshPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::MeshPhrase, smtk::model::DescriptivePhrase > instance(m, "MeshPhrase");
  instance
    .def(py::init<::smtk::model::MeshPhrase const &>())
    .def("deepcopy", (smtk::model::MeshPhrase & (smtk::model::MeshPhrase::*)(::smtk::model::MeshPhrase const &)) &smtk::model::MeshPhrase::operator=)
    .def("classname", &smtk::model::MeshPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::MeshPhrase> (*)()) &smtk::model::MeshPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::MeshPhrase> (*)(::std::shared_ptr<smtk::model::MeshPhrase> &)) &smtk::model::MeshPhrase::create, py::arg("ref"))
    .def("isCollection", &smtk::model::MeshPhrase::isCollection)
    .def("isRelatedColorMutable", &smtk::model::MeshPhrase::isRelatedColorMutable)
    .def("isTitleMutable", &smtk::model::MeshPhrase::isTitleMutable)
    .def("relatedColor", &smtk::model::MeshPhrase::relatedColor)
    .def("relatedMesh", &smtk::model::MeshPhrase::relatedMesh)
    .def("relatedMeshCollection", &smtk::model::MeshPhrase::relatedMeshCollection)
    .def("setMutability", &smtk::model::MeshPhrase::setMutability, py::arg("whatsMutable"))
    .def("setRelatedColor", &smtk::model::MeshPhrase::setRelatedColor, py::arg("rgba"))
    .def("setTitle", &smtk::model::MeshPhrase::setTitle, py::arg("newTitle"))
    .def("setup", (smtk::model::MeshPhrase::Ptr (smtk::model::MeshPhrase::*)(::smtk::mesh::MeshSet const &, ::smtk::model::DescriptivePhrase::Ptr)) &smtk::model::MeshPhrase::setup, py::arg("meshset"), py::arg("parent") = ::smtk::model::DescriptivePhrasePtr( ))
    .def("setup", (smtk::model::MeshPhrase::Ptr (smtk::model::MeshPhrase::*)(::smtk::mesh::CollectionPtr const &, ::smtk::model::DescriptivePhrase::Ptr)) &smtk::model::MeshPhrase::setup, py::arg("meshes"), py::arg("parent") = ::smtk::model::DescriptivePhrasePtr( ))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::MeshPhrase> (smtk::model::MeshPhrase::*)() const) &smtk::model::MeshPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::MeshPhrase> (smtk::model::MeshPhrase::*)()) &smtk::model::MeshPhrase::shared_from_this)
    .def("title", &smtk::model::MeshPhrase::title)
    .def("updateMesh", (void (smtk::model::MeshPhrase::*)(::smtk::mesh::MeshSet const &)) &smtk::model::MeshPhrase::updateMesh, py::arg("meshset"))
    .def("updateMesh", (void (smtk::model::MeshPhrase::*)(::smtk::mesh::CollectionPtr const &)) &smtk::model::MeshPhrase::updateMesh, py::arg("c"))
    ;
  return instance;
}

#endif
