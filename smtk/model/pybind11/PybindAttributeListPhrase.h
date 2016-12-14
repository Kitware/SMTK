//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_AttributeListPhrase_h
#define pybind_smtk_model_AttributeListPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/AttributeListPhrase.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::AttributeListPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_AttributeListPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::AttributeListPhrase, smtk::model::DescriptivePhrase > instance(m, "AttributeListPhrase");
  instance
    .def(py::init<::smtk::model::AttributeListPhrase const &>())
    .def("deepcopy", (smtk::model::AttributeListPhrase & (smtk::model::AttributeListPhrase::*)(::smtk::model::AttributeListPhrase const &)) &smtk::model::AttributeListPhrase::operator=)
    .def("classname", &smtk::model::AttributeListPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::AttributeListPhrase> (*)()) &smtk::model::AttributeListPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::AttributeListPhrase> (*)(::std::shared_ptr<smtk::model::AttributeListPhrase> &)) &smtk::model::AttributeListPhrase::create, py::arg("ref"))
    .def("relatedEntity", &smtk::model::AttributeListPhrase::relatedEntity)
    .def("relatedEntityId", &smtk::model::AttributeListPhrase::relatedEntityId)
    // .def("setup", (smtk::model::AttributeListPhrase::Ptr (smtk::model::AttributeListPhrase::*)(::smtk::model::EntityRef const &, ::smtk::model::DescriptivePhrasePtr)) &smtk::model::AttributeListPhrase::setup, py::arg("ent"), py::arg("parent"))
    .def("setup", (smtk::model::AttributeListPhrase::Ptr (smtk::model::AttributeListPhrase::*)(::smtk::model::EntityRef const &, ::smtk::model::AttributeSet const &, ::smtk::model::DescriptivePhrasePtr)) &smtk::model::AttributeListPhrase::setup, py::arg("ent"), py::arg("subset"), py::arg("parent"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::AttributeListPhrase> (smtk::model::AttributeListPhrase::*)() const) &smtk::model::AttributeListPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::AttributeListPhrase> (smtk::model::AttributeListPhrase::*)()) &smtk::model::AttributeListPhrase::shared_from_this)
    .def("subtitle", &smtk::model::AttributeListPhrase::subtitle)
    .def("title", &smtk::model::AttributeListPhrase::title)
    ;
  return instance;
}

#endif
