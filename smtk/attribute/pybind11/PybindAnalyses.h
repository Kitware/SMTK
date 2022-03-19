//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_attribute_Analyses_h
#define pybind_smtk_attribute_Analyses_h

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "smtk/attribute/Analyses.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"

namespace py = pybind11;

inline py::class_< smtk::attribute::Analyses > pybind11_init_smtk_attribute_Analyses(py::module &m)
{
  py::class_< smtk::attribute::Analyses, std::unique_ptr<smtk::attribute::Analyses, py::nodelete>> instance(m, "Analyses");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::attribute::Analyses const &>())
    .def("deepcopy", (smtk::attribute::Analyses & (smtk::attribute::Analyses::*)(::smtk::attribute::Analyses const &)) &smtk::attribute::Analyses::operator=)
    .def("create", &smtk::attribute::Analyses::create, py::arg("name"))
    .def("find", &smtk::attribute::Analyses::find, py::arg("name"))
    .def("topLevel", &smtk::attribute::Analyses::topLevel)
    .def("analyses", &smtk::attribute::Analyses::analyses)
    .def("size", &smtk::attribute::Analyses::size)
    .def("setTopLevelExclusive", &smtk::attribute::Analyses::setTopLevelExclusive, py::arg("mode"))
    .def("areTopLevelExclusive", &smtk::attribute::Analyses::areTopLevelExclusive)
    .def("setAnalysisParent", &smtk::attribute::Analyses::setAnalysisParent, py::arg("analysis"), py::arg("parent"))
    .def("buildAnalysesDefinition", &smtk::attribute::Analyses::buildAnalysesDefinition, py::arg("resource"), py::arg("type"), py::arg("label") = "Analysis")
    .def("getAnalysisAttributeCategories", (std::set<std::string> (::smtk::attribute::Analyses::*)(smtk::attribute::ConstAttributePtr attribute)) &smtk::attribute::Analyses::getAnalysisAttributeCategories)
    ;
  py::class_< smtk::attribute::Analyses::Analysis, std::unique_ptr<smtk::attribute::Analyses::Analysis, py::nodelete>>(instance, "Analysis")
    .def(py::init<::smtk::attribute::Analyses::Analysis const &>())
    .def("deepcopy", (smtk::attribute::Analyses::Analysis & (smtk::attribute::Analyses::Analysis::*)(::smtk::attribute::Analyses::Analysis const &)) &smtk::attribute::Analyses::Analysis::operator=)
    .def("name", &smtk::attribute::Analyses::Analysis::name)
    .def("setLocalCategories", &smtk::attribute::Analyses::Analysis::setLocalCategories, py::arg("cats"))
    .def("localCategories", &smtk::attribute::Analyses::Analysis::localCategories)
    .def("categories", &smtk::attribute::Analyses::Analysis::categories)
    .def("parent", &smtk::attribute::Analyses::Analysis::parent)
    .def("setParent", &smtk::attribute::Analyses::Analysis::setParent, py::arg("p"))
    .def("setExclusive", &smtk::attribute::Analyses::Analysis::setExclusive, py::arg("mode"))
    .def("isExclusive", &smtk::attribute::Analyses::Analysis::isExclusive)
    .def("setRequired", &smtk::attribute::Analyses::Analysis::setRequired, py::arg("mode"))
    .def("isRequired", &smtk::attribute::Analyses::Analysis::isRequired)
    .def("children", &smtk::attribute::Analyses::Analysis::children)
    .def("buildAnalysisItem", (void (smtk::attribute::Analyses::Analysis::*)(::smtk::attribute::DefinitionPtr &) const) &smtk::attribute::Analyses::Analysis::buildAnalysisItem, py::arg("sitem"))
    .def("buildAnalysisItem", (void (smtk::attribute::Analyses::Analysis::*)(::smtk::attribute::GroupItemDefinitionPtr &) const) &smtk::attribute::Analyses::Analysis::buildAnalysisItem, py::arg("gitem"))
    .def("buildAnalysisItem", (void (smtk::attribute::Analyses::Analysis::*)(::smtk::attribute::StringItemDefinitionPtr &) const) &smtk::attribute::Analyses::Analysis::buildAnalysisItem, py::arg("sitem"))
    ;
  return instance;
}

#endif
