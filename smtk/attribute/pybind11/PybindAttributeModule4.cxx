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

#include "smtk/attribute/Resource.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Definition.h"

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindAssociate.h"
#include "PybindDissociate.h"
#include "PybindExport.h"
#include "PybindImport.h"
#include "PybindRead.h"
#include "PybindWrite.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void attributePart4(py::module& attribute)
{
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::module::import("smtk.operation");

  PySharedPtrClass< smtk::attribute::Associate, smtk::operation::XMLOperation > smtk_attribute_Associate = pybind11_init_smtk_attribute_Associate(attribute);
  PySharedPtrClass< smtk::attribute::Dissociate, smtk::operation::XMLOperation > smtk_attribute_Dissociate = pybind11_init_smtk_attribute_Dissociate(attribute);
  PySharedPtrClass< smtk::attribute::Import, smtk::operation::XMLOperation > smtk_attribute_Import = pybind11_init_smtk_attribute_Import(attribute);
  PySharedPtrClass< smtk::attribute::Export, smtk::operation::XMLOperation > smtk_attribute_Export = pybind11_init_smtk_attribute_Export(attribute);
  PySharedPtrClass< smtk::attribute::Read, smtk::operation::XMLOperation > smtk_attribute_Read = pybind11_init_smtk_attribute_Read(attribute);
  PySharedPtrClass< smtk::attribute::Write, smtk::operation::XMLOperation > smtk_attribute_Write = pybind11_init_smtk_attribute_Write(attribute);
}
