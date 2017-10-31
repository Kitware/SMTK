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

#include "PybindDescriptivePhrase.h"
#include "PybindSubphraseGenerator.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(view, m)
{
  m.doc() = "<description>";
  py::module smtk = m.def_submodule("smtk", "<description>");
  py::module view = smtk.def_submodule("view", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::view::DescriptivePhrase > smtk_view_DescriptivePhrase = pybind11_init_smtk_view_DescriptivePhrase(view);
  PySharedPtrClass< smtk::view::SubphraseGenerator > smtk_view_SubphraseGenerator = pybind11_init_smtk_view_SubphraseGenerator(view);
  pybind11_init_smtk_view_DescriptivePhraseType(view);
}
