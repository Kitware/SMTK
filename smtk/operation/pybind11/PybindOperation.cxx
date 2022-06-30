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
#include <utility>
SMTK_THIRDPARTY_POST_INCLUDE

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindMetadata.h"
#include "PybindMetadataContainer.h"
#include "PybindMetadataObserver.h"
#include "PybindOperation.h"
#include "PybindManager.h"
#include "PybindMarkGeometry.h"
#include "PybindObserver.h"
#include "PybindXMLOperation.h"

#include "PybindReadResource.h"
#include "PybindRemoveResource.h"
#include "PybindSetProperty.h"
#include "PybindWriteResource.h"

#include "PybindRegistrar.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindOperation, operation)
{
  operation.doc() = "<description>";

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::operation::IndexTag > smtk_operation_IndexTag = pybind11_init_smtk_operation_IndexTag(operation);
  py::class_< smtk::operation::Metadata > smtk_operation_Metadata = pybind11_init_smtk_operation_Metadata(operation);
  py::class_< smtk::operation::MetadataObservers > smtk_operation_MetadataObservers = pybind11_init_smtk_operation_MetadataObservers(operation);
  py::class_< smtk::operation::NameTag > smtk_operation_NameTag = pybind11_init_smtk_operation_NameTag(operation);
  PySharedPtrClass< smtk::operation::Manager > smtk_operation_Manager = pybind11_init_smtk_operation_Manager(operation);
  auto smtk_operation_MarkGeometry = pybind11_init_smtk_operation_MarkGeometry(operation);
  PySharedPtrClass< smtk::operation::Operation, smtk::operation::PyOperation > smtk_operation_Operation = pybind11_init_smtk_operation_Operation(operation);
  py::class_< smtk::operation::Observers > smtk_operation_Observers = pybind11_init_smtk_operation_Observers(operation);
  pybind11_init_smtk_operation_EventType(operation);
  PySharedPtrClass< smtk::operation::XMLOperation, smtk::operation::Operation > smtk_operation_XMLOperation = pybind11_init_smtk_operation_XMLOperation(operation);

  PySharedPtrClass< smtk::operation::ReadResource, smtk::operation::XMLOperation > smtk_operation_ReadResource = pybind11_init_smtk_operation_ReadResource(operation);
  PySharedPtrClass< smtk::operation::RemoveResource, smtk::operation::XMLOperation > smtk_operation_RemoveResource = pybind11_init_smtk_operation_RemoveResource(operation);
  PySharedPtrClass< smtk::operation::SetProperty, smtk::operation::XMLOperation > smtk_operation_SetProperty = pybind11_init_smtk_operation_SetProperty(operation);
  PySharedPtrClass< smtk::operation::WriteResource, smtk::operation::XMLOperation > smtk_operation_WriteResource = pybind11_init_smtk_operation_WriteResource(operation);

  py::class_< smtk::operation::Registrar > smtk_operation_Registrar = pybind11_init_smtk_operation_Registrar(operation);
}
