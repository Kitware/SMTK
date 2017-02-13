//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_io_XmlV3StringWriter_h
#define pybind_smtk_io_XmlV3StringWriter_h

#include <pybind11/pybind11.h>

#include "smtk/io/XmlV3StringWriter.h"

#include "smtk/attribute/System.h"
#include "smtk/io/XmlV2StringWriter.h"

namespace py = pybind11;

PySharedPtrClass< smtk::io::XmlV3StringWriter, smtk::io::XmlV2StringWriter > pybind11_init_smtk_io_XmlV3StringWriter(py::module &m)
{
  PySharedPtrClass< smtk::io::XmlV3StringWriter, smtk::io::XmlV2StringWriter > instance(m, "XmlV3StringWriter");
  instance
    .def(py::init<::smtk::io::XmlV3StringWriter const &>())
    .def(py::init<::smtk::attribute::System const &>())
    ;
  return instance;
}

#endif
