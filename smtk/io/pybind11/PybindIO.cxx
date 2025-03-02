//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"

#include "PybindAttributeReader.h"
#include "PybindAttributeWriter.h"
#include "PybindLogger.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindIO, io)
{
  io.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::io::AttributeReader > smtk_io_AttributeReader = pybind11_init_smtk_io_AttributeReader(io);
  PySharedPtrClass< smtk::io::AttributeWriter > smtk_io_AttributeWriter = pybind11_init_smtk_io_AttributeWriter(io);
  PySharedPtrClass< smtk::io::Logger > smtk_io_Logger = pybind11_init_smtk_io_Logger(io);
}
