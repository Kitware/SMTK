//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_attribute_ValueItemDefinitionTemplate_h
#define pybind_attribute_ValueItemDefinitionTemplate_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include "smtk/common/DateTimeZonePair.h"

namespace py = pybind11;

PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<int>, smtk::attribute::ValueItemDefinition> pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_int_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<int>, smtk::attribute::ValueItemDefinition> instance(m, "ValueItemDefinitionTemplate_int_");
  instance
    .def("maxRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::maxRange)
    .def("minRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::minRange)
    .def("setMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::setMaxRange)
    .def("setMinRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::setMinRange)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<int>::*)(int const &)) &smtk::attribute::ValueItemDefinitionTemplate<int>::addDiscreteValue)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<int>::*)(int const &, ::std::string const &)) &smtk::attribute::ValueItemDefinitionTemplate<int>::addDiscreteValue)
    .def("clearRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::clearRange)
    .def("defaultValue", (int const & (smtk::attribute::ValueItemDefinitionTemplate<int>::*)() const) &smtk::attribute::ValueItemDefinitionTemplate<int>::defaultValue)
    .def("defaultValue", (int const & (smtk::attribute::ValueItemDefinitionTemplate<int>::*)(::size_t) const) &smtk::attribute::ValueItemDefinitionTemplate<int>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemDefinitionTemplate<int>::defaultValues)
    .def("discreteValue", &smtk::attribute::ValueItemDefinitionTemplate<int>::discreteValue, py::arg("element"))
    .def("findDiscreteIndex", &smtk::attribute::ValueItemDefinitionTemplate<int>::findDiscreteIndex, py::arg("val"))
    .def("hasMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::hasMaxRange)
    .def("hasMinRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::hasMinRange)
    .def("hasRange", &smtk::attribute::ValueItemDefinitionTemplate<int>::hasRange)
    .def("isValueValid", &smtk::attribute::ValueItemDefinitionTemplate<int>::isValueValid, py::arg("val"))
    .def("maxRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<int>::maxRangeInclusive)
    .def("minRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<int>::minRangeInclusive)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<int>::*)(int const &)) &smtk::attribute::ValueItemDefinitionTemplate<int>::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<int>::*)(::std::vector<int, std::allocator<int> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<int>::setDefaultValue)
    ;
  return instance;
}

PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<double>, smtk::attribute::ValueItemDefinition> pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_double_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<double>, smtk::attribute::ValueItemDefinition> instance(m, "ValueItemDefinitionTemplate_double_");
  instance
    .def("maxRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::maxRange)
    .def("minRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::minRange)
    .def("setMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::setMaxRange)
    .def("setMinRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::setMinRange)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<double>::*)(double const &)) &smtk::attribute::ValueItemDefinitionTemplate<double>::addDiscreteValue)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<double>::*)(double const &, ::std::string const &)) &smtk::attribute::ValueItemDefinitionTemplate<double>::addDiscreteValue)
    .def("clearRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::clearRange)
    .def("defaultValue", (double const & (smtk::attribute::ValueItemDefinitionTemplate<double>::*)() const) &smtk::attribute::ValueItemDefinitionTemplate<double>::defaultValue)
    .def("defaultValue", (double const & (smtk::attribute::ValueItemDefinitionTemplate<double>::*)(::size_t) const) &smtk::attribute::ValueItemDefinitionTemplate<double>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemDefinitionTemplate<double>::defaultValues)
    .def("discreteValue", &smtk::attribute::ValueItemDefinitionTemplate<double>::discreteValue, py::arg("element"))
    .def("findDiscreteIndex", &smtk::attribute::ValueItemDefinitionTemplate<double>::findDiscreteIndex, py::arg("val"))
    .def("hasMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::hasMaxRange)
    .def("hasMinRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::hasMinRange)
    .def("hasRange", &smtk::attribute::ValueItemDefinitionTemplate<double>::hasRange)
    .def("isValueValid", &smtk::attribute::ValueItemDefinitionTemplate<double>::isValueValid, py::arg("val"))
    .def("maxRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<double>::maxRangeInclusive)
    .def("minRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<double>::minRangeInclusive)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<double>::*)(double const &)) &smtk::attribute::ValueItemDefinitionTemplate<double>::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<double>::*)(::std::vector<double, std::allocator<double> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<double>::setDefaultValue)
    ;
  return instance;
}

PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<std::string>, smtk::attribute::ValueItemDefinition> pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_string_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<std::string>, smtk::attribute::ValueItemDefinition> instance(m, "ValueItemDefinitionTemplate_string_");
  instance
    .def("maxRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::maxRange)
    .def("minRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::minRange)
    .def("setMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::setMaxRange)
    .def("setMinRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::setMinRange)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)(::std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::addDiscreteValue)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)(::std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &, ::std::string const &)) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::addDiscreteValue)
    .def("clearRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::clearRange)
    .def("defaultValue", (std::basic_string<char, std::char_traits<char>, std::allocator<char> > const & (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)() const) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::defaultValue)
    .def("defaultValue", (std::basic_string<char, std::char_traits<char>, std::allocator<char> > const & (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)(::size_t) const) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::defaultValues)
    .def("discreteValue", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::discreteValue, py::arg("element"))
    .def("findDiscreteIndex", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::findDiscreteIndex, py::arg("val"))
    .def("hasMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::hasMaxRange)
    .def("hasMinRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::hasMinRange)
    .def("hasRange", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::hasRange)
    .def("isValueValid", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::isValueValid, py::arg("val"))
    .def("maxRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::maxRangeInclusive)
    .def("minRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<std::string>::minRangeInclusive)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)(::std::basic_string<char, std::char_traits<char>, std::allocator<char> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<std::string>::*)(::std::vector<std::basic_string<char>, std::allocator<std::string> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<std::string>::setDefaultValue)
    ;
  return instance;
}

PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>, smtk::attribute::ValueItemDefinition> pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_datetime_(py::module &m)
{
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>, smtk::attribute::ValueItemDefinition> instance(m, "ValueItemDefinitionTemplate_datetime_");
  instance
    .def("maxRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::maxRange)
    .def("minRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::minRange)
    .def("setMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::setMaxRange)
    .def("setMinRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::setMinRange)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)(smtk::common::DateTimeZonePair const &)) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::addDiscreteValue)
    .def("addDiscreteValue", (void (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)(smtk::common::DateTimeZonePair const &, ::std::string const &)) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::addDiscreteValue)
    .def("clearRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::clearRange)
    .def("defaultValue", (smtk::common::DateTimeZonePair const & (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)() const) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::defaultValue)
    .def("defaultValue", (smtk::common::DateTimeZonePair const & (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)(::size_t) const) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::defaultValue)
    .def("defaultValues", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::defaultValues)
    .def("discreteValue", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::discreteValue, py::arg("element"))
    .def("findDiscreteIndex", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::findDiscreteIndex, py::arg("val"))
    .def("hasMaxRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::hasMaxRange)
    .def("hasMinRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::hasMinRange)
    .def("hasRange", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::hasRange)
    .def("isValueValid", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::isValueValid, py::arg("val"))
    .def("maxRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::maxRangeInclusive)
    .def("minRangeInclusive", &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::minRangeInclusive)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)(smtk::common::DateTimeZonePair const &)) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::setDefaultValue)
    .def("setDefaultValue", (bool (smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::*)(::std::vector<smtk::common::DateTimeZonePair, std::allocator<::smtk::common::DateTimeZonePair> > const &)) &smtk::attribute::ValueItemDefinitionTemplate<::smtk::common::DateTimeZonePair>::setDefaultValue)
    ;
  return instance;
}

#endif
