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
#include <pybind11/functional.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindResource.h"
#include "PybindPropertyType.h"
#include "PybindComponent.h"
#include "PybindManager.h"
#include "PybindObserver.h"
#include "PybindPersistentObject.h"
#include "PybindSet.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindResource, resource)
{
  resource.doc() = "<description>";

  pybind11_init_smtk_resource_PropertyType(resource);

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::resource::PersistentObject > smtk_resource_PersistentObject = pybind11_init_smtk_resource_PersistentObject(resource);
  py::class_< smtk::resource::Resource > smtk_resource_Resource = pybind11_init_smtk_resource_Resource(resource);
  py::class_< smtk::resource::Component > smtk_resource_Component = pybind11_init_smtk_resource_Component(resource);
  py::class_< smtk::resource::Manager > smtk_resource_Manager = pybind11_init_smtk_resource_Manager(resource);
  py::class_< smtk::resource::Observers > smtk_resource_Observers = pybind11_init_smtk_resource_Observers(resource);
  pybind11_init_smtk_resource_EventType(resource);
  py::class_< smtk::resource::Set > smtk_resource_Set = pybind11_init_smtk_resource_Set(resource);
}
