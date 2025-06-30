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
#include "PybindHandleManager.h"
#include "PybindSubphraseGenerator.h"
#include "PybindSelection.h"
#include "PybindSelectionObserver.h"
#include "PybindView.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindView, view)
{
  view.doc() =
    "Present SMTK resources to users.\n"
    "\n"
    "These classes provide abstractions for creating\n"
    "views of SMTK resources for presentation to users\n"
    "without adding a dependency on any particular UI\n"
    "library."
    ;
  pybind11_init_smtk_view_SelectionAction(view);
  pybind11_init_smtk_view_DescriptivePhraseType(view);

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::view::DescriptivePhrase > smtk_view_DescriptivePhrase = pybind11_init_smtk_view_DescriptivePhrase(view);
  PySharedPtrClass< smtk::view::SubphraseGenerator > smtk_view_SubphraseGenerator = pybind11_init_smtk_view_SubphraseGenerator(view);

  PySharedPtrClass< smtk::view::Configuration > smtk_view_View = pybind11_init_smtk_view_View(view);
  py::class_< smtk::view::SelectionObservers > smtk_view_SelectionObserver = pybind11_init_smtk_view_SelectionObservers(view);
  py::class_< smtk::view::Selection > smtk_view_Selection = pybind11_init_smtk_view_Selection(view);
  PySharedPtrClass<smtk::view::HandleManager> smtk_view_HandleManager =  pybind11_init_smtk_view_HandleManager(view);
  py::class_<smtk::view::HandleManager::Observers> smtk_view_HandleManager_Observers = pybind11_init_smtk_view_HandleManager_Observers(view);
  py::class_<smtk::view::HandleManager::Observers::Key> smtk_view_HandleManager_Observers_Key = pybind11_init_smtk_view_HandleManager_Observers_Key(view);

}
