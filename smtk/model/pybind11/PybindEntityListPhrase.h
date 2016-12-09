//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_EntityListPhrase_h
#define pybind_smtk_model_EntityListPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/EntityListPhrase.h"

#include "smtk/model/DescriptivePhrase.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::EntityListPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_EntityListPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::EntityListPhrase, smtk::model::DescriptivePhrase > instance(m, "EntityListPhrase");
  instance
    .def(py::init<::smtk::model::EntityListPhrase const &>())
    .def("deepcopy", (smtk::model::EntityListPhrase & (smtk::model::EntityListPhrase::*)(::smtk::model::EntityListPhrase const &)) &smtk::model::EntityListPhrase::operator=)
    .def("classname", &smtk::model::EntityListPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::EntityListPhrase> (*)()) &smtk::model::EntityListPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::EntityListPhrase> (*)(::std::shared_ptr<smtk::model::EntityListPhrase> &)) &smtk::model::EntityListPhrase::create, py::arg("ref"))
    .def("relatedEntities", (smtk::model::EntityRefArray (smtk::model::EntityListPhrase::*)() const) &smtk::model::EntityListPhrase::relatedEntities)
    .def("relatedEntities", (smtk::model::EntityRefArray & (smtk::model::EntityListPhrase::*)()) &smtk::model::EntityListPhrase::relatedEntities)
    .def("setFlags", &smtk::model::EntityListPhrase::setFlags, py::arg("commonFlags"), py::arg("unionFlags"))
    .def("setup", [](smtk::model::EntityListPhrase& p, const std::vector<smtk::model::EntityRef> entities) { return p.setup(entities); })
    .def("setup", [](smtk::model::EntityListPhrase& p, const std::vector<smtk::model::EntityRef> entities, smtk::model::DescriptivePhrase::Ptr parent) { return p.setup(entities, parent); })
    .def("shared_from_this", (std::shared_ptr<const smtk::model::EntityListPhrase> (smtk::model::EntityListPhrase::*)() const) &smtk::model::EntityListPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::EntityListPhrase> (smtk::model::EntityListPhrase::*)()) &smtk::model::EntityListPhrase::shared_from_this)
    .def("subtitle", &smtk::model::EntityListPhrase::subtitle)
    .def("title", &smtk::model::EntityListPhrase::title)
    ;
  return instance;
}

#endif
