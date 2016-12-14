//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_MeshListPhrase_h
#define pybind_smtk_model_MeshListPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/MeshListPhrase.h"

#include "smtk/model/DescriptivePhrase.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::MeshListPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_MeshListPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::MeshListPhrase, smtk::model::DescriptivePhrase > instance(m, "MeshListPhrase");
  instance
    .def(py::init<::smtk::model::MeshListPhrase const &>())
    .def("deepcopy", (smtk::model::MeshListPhrase & (smtk::model::MeshListPhrase::*)(::smtk::model::MeshListPhrase const &)) &smtk::model::MeshListPhrase::operator=)
    .def("classname", &smtk::model::MeshListPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::MeshListPhrase> (*)()) &smtk::model::MeshListPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::MeshListPhrase> (*)(::std::shared_ptr<smtk::model::MeshListPhrase> &)) &smtk::model::MeshListPhrase::create, py::arg("ref"))
    .def("relatedCollections", &smtk::model::MeshListPhrase::relatedCollections)
    .def("relatedMeshes", &smtk::model::MeshListPhrase::relatedMeshes)
    .def("setup", (smtk::model::MeshListPhrase::Ptr (smtk::model::MeshListPhrase::*)(::std::vector<smtk::mesh::MeshSet, std::allocator<smtk::mesh::MeshSet> > const &, ::smtk::model::DescriptivePhrase::Ptr)) &smtk::model::MeshListPhrase::setup, py::arg("arg0"), py::arg("parent") = ::smtk::model::DescriptivePhrasePtr( ))
    .def("setup", (smtk::model::MeshListPhrase::Ptr (smtk::model::MeshListPhrase::*)(::std::vector<std::shared_ptr<smtk::mesh::Collection>, std::allocator<std::shared_ptr<smtk::mesh::Collection> > > const &, ::smtk::model::DescriptivePhrase::Ptr)) &smtk::model::MeshListPhrase::setup, py::arg("arg0"), py::arg("parent") = ::smtk::model::DescriptivePhrasePtr( ))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::MeshListPhrase> (smtk::model::MeshListPhrase::*)() const) &smtk::model::MeshListPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::MeshListPhrase> (smtk::model::MeshListPhrase::*)()) &smtk::model::MeshListPhrase::shared_from_this)
    .def("subtitle", &smtk::model::MeshListPhrase::subtitle)
    .def("title", &smtk::model::MeshListPhrase::title)
    ;
  return instance;
}

#endif
