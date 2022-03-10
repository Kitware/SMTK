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

void attributePart1(py::module& attribute);
void attributePart2(py::module& attribute);
void attributePart3(py::module& attribute);
void attributePart4(py::module& attribute);

PYBIND11_MODULE(_smtkPybindAttribute, attribute)
{
  attribute.doc() = "<description>";

  py::module::import("smtk.common");
  py::module::import("smtk.resource");

  attributePart1(attribute);
  attributePart2(attribute);
  attributePart3(attribute);
  attributePart4(attribute);
}
