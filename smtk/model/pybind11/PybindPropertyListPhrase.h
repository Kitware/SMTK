//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_PropertyListPhrase_h
#define pybind_smtk_model_PropertyListPhrase_h

#include <pybind11/pybind11.h>

#include "smtk/model/PropertyListPhrase.h"

#include "smtk/common/UUID.h"
#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/PropertyType.h"

namespace py = pybind11;

PySharedPtrClass< smtk::model::PropertyListPhrase, smtk::model::DescriptivePhrase > pybind11_init_smtk_model_PropertyListPhrase(py::module &m)
{
  PySharedPtrClass< smtk::model::PropertyListPhrase, smtk::model::DescriptivePhrase > instance(m, "PropertyListPhrase");
  instance
    .def(py::init<::smtk::model::PropertyListPhrase const &>())
    .def("deepcopy", (smtk::model::PropertyListPhrase & (smtk::model::PropertyListPhrase::*)(::smtk::model::PropertyListPhrase const &)) &smtk::model::PropertyListPhrase::operator=)
    .def("classname", &smtk::model::PropertyListPhrase::classname)
    .def_static("create", (std::shared_ptr<smtk::model::PropertyListPhrase> (*)()) &smtk::model::PropertyListPhrase::create)
    .def_static("create", (std::shared_ptr<smtk::model::PropertyListPhrase> (*)(::std::shared_ptr<smtk::model::PropertyListPhrase> &)) &smtk::model::PropertyListPhrase::create, py::arg("ref"))
    .def("propertyNames", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > & (smtk::model::PropertyListPhrase::*)()) &smtk::model::PropertyListPhrase::propertyNames)
    .def("propertyNames", (std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const & (smtk::model::PropertyListPhrase::*)() const) &smtk::model::PropertyListPhrase::propertyNames)
    .def_static("propertyToPhraseType", &smtk::model::PropertyListPhrase::propertyToPhraseType, py::arg("p"))
    .def("relatedEntity", &smtk::model::PropertyListPhrase::relatedEntity)
    .def("relatedEntityId", &smtk::model::PropertyListPhrase::relatedEntityId)
    .def("relatedPropertyType", &smtk::model::PropertyListPhrase::relatedPropertyType)
    .def("setup", (smtk::model::PropertyListPhrase::Ptr (smtk::model::PropertyListPhrase::*)(::smtk::model::EntityRef const &, ::smtk::model::PropertyType, ::smtk::model::DescriptivePhrasePtr)) &smtk::model::PropertyListPhrase::setup, py::arg("entity"), py::arg("ptype"), py::arg("parent"))
    .def("setup", (smtk::model::PropertyListPhrase::Ptr (smtk::model::PropertyListPhrase::*)(::smtk::model::EntityRef const &, ::smtk::model::PropertyType, ::std::set<std::basic_string<char>, std::less<std::basic_string<char> >, std::allocator<std::basic_string<char> > > const &, ::smtk::model::DescriptivePhrasePtr)) &smtk::model::PropertyListPhrase::setup, py::arg("entity"), py::arg("ptype"), py::arg("pnames"), py::arg("parent"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::PropertyListPhrase> (smtk::model::PropertyListPhrase::*)() const) &smtk::model::PropertyListPhrase::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::PropertyListPhrase> (smtk::model::PropertyListPhrase::*)()) &smtk::model::PropertyListPhrase::shared_from_this)
    .def("subtitle", &smtk::model::PropertyListPhrase::subtitle)
    .def("title", &smtk::model::PropertyListPhrase::title)
    ;
  return instance;
}

#endif
