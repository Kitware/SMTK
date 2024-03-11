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
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindComponent.h"
#include "PybindCopyOptions.h"
#include "PybindManager.h"
#include "PybindObserver.h"
#include "PybindPersistentObject.h"
#include "PybindProperties.h"
#include "PybindPropertyType.h"
#include "PybindResource.h"
#include "PybindRegistrar.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindResource, resource)
{
  resource.doc() = "<description>";

  pybind11_init_smtk_resource_PropertyType(resource);

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  auto smtk_resource_CopyOptions = pybind11_init_smtk_resource_CopyOptions(resource);
  auto smtk_resource_PersistentObject = pybind11_init_smtk_resource_PersistentObject(resource);
  auto smtk_resource_Properties = pybind11_init_smtk_resource_Properties(resource);
  auto smtk_resource_Resource = pybind11_init_smtk_resource_Resource(resource);
  auto smtk_resource_Component = pybind11_init_smtk_resource_Component(resource);

  // Declare each type of property we support. See smtk::resource::ResourceProperties::PropertyTypes
  // for the list of predefined property types. If you have a plugin that wants other types supported,
  // you must add them to this module as these are added:
  auto smtk_resource_PropertiesOfTypeBool = pybind11_init_smtk_resource_PropertiesOfType<bool>(resource);
  auto smtk_resource_PropertiesOfTypeInt = pybind11_init_smtk_resource_PropertiesOfType<int>(resource);
  auto smtk_resource_PropertiesOfTypeLong = pybind11_init_smtk_resource_PropertiesOfType<long>(resource);
  auto smtk_resource_PropertiesOfTypeDouble = pybind11_init_smtk_resource_PropertiesOfType<double>(resource);
  auto smtk_resource_PropertiesOfTypeString = pybind11_init_smtk_resource_PropertiesOfType<std::string>(resource);

  auto smtk_resource_PropertiesOfTypeVecBool = pybind11_init_smtk_resource_PropertiesOfType<std::vector<bool>>(resource);
  auto smtk_resource_PropertiesOfTypeVecInt = pybind11_init_smtk_resource_PropertiesOfType<std::vector<int>>(resource);
  auto smtk_resource_PropertiesOfTypeVecLong = pybind11_init_smtk_resource_PropertiesOfType<std::vector<long>>(resource);
  auto smtk_resource_PropertiesOfTypeVecDouble = pybind11_init_smtk_resource_PropertiesOfType<std::vector<double>>(resource);
  auto smtk_resource_PropertiesOfTypeVecString = pybind11_init_smtk_resource_PropertiesOfType<std::vector<std::string>>(resource);

  auto smtk_resource_Manager = pybind11_init_smtk_resource_Manager(resource);
  auto smtk_resource_Observers = pybind11_init_smtk_resource_Observers(resource);
  pybind11_init_smtk_resource_EventType(resource);

  auto smtk_resource_Registrar = pybind11_init_smtk_resource_Registrar(resource);
}
