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

#include "PybindToken.h"
#include "PybindManager.h"

PYBIND11_MODULE(_smtkPybindString, string)
{
  string.doc() = "<description>";

  py::module::import("smtk.common");

  auto smtk_string_Token = pybind11_init_smtk_string_Token(string);
  auto smtk_string_Manager = pybind11_init_smtk_string_Manager(string);
}
