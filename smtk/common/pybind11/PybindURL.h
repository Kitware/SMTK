//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_URL_h
#define pybind_smtk_common_URL_h

#include <pybind11/pybind11.h>

#include "smtk/common/URL.h"

namespace py = pybind11;

inline py::class_< smtk::common::URL > pybind11_init_smtk_common_URL(py::module &m)
{
  py::class_< smtk::common::URL > instance(m, "URL");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::URL const &>())
    .def(py::init<::std::string const &>())
    .def("deepcopy",
      (smtk::common::URL & (smtk::common::URL::*)(::smtk::common::URL const &))
      &smtk::common::URL::operator=)
    .def("valid", &smtk::common::URL::valid)
    .def("to_string", [](const smtk::common::URL& url)
      {
        auto result = (std::string)url;
        return result;
      }
    )
    .def("scheme", (smtk::string::Token (smtk::common::URL::*)() const)&smtk::common::URL::scheme)
    .def("authority", (smtk::string::Token (smtk::common::URL::*)() const)&smtk::common::URL::authority)
    .def("path", (smtk::string::Token (smtk::common::URL::*)() const)&smtk::common::URL::path)
    .def("query", (smtk::string::Token (smtk::common::URL::*)() const)&smtk::common::URL::query)
    .def("fragment", (smtk::string::Token (smtk::common::URL::*)() const)&smtk::common::URL::fragment)
    .def("setScheme", [](smtk::common::URL* url, const std::string& txt) { return url->setScheme(txt); }, py::arg("scheme"))
    .def("setAuthority", [](smtk::common::URL* url, const std::string& txt) { return url->setAuthority(txt); }, py::arg("authority"))
    .def("setPath", [](smtk::common::URL* url, const std::string& txt) { return url->setPath(txt); }, py::arg("path"))
    .def("setQuery", [](smtk::common::URL* url, const std::string& txt) { return url->setQuery(txt); }, py::arg("query"))
    .def("setFragment", [](smtk::common::URL* url, const std::string& txt) { return url->setFragment(txt); }, py::arg("fragment"))
    ;
  return instance;
}

#endif
