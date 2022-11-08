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

#include "PybindAnalyses.h"
#include "PybindAttribute.h"
#include "PybindCopyAssignmentOptions.h"
#include "PybindDefinition.h"
#include "PybindFileItem.h"
#include "PybindFileItemDefinition.h"
#include "PybindFileSystemItem.h"
#include "PybindFileSystemItemDefinition.h"
#include "PybindGroupItem.h"
#include "PybindGroupItemDefinition.h"
#include "PybindItem.h"
#include "PybindItemDefinition.h"
#include "PybindQueries.h"
#include "PybindReferenceItem.h"
#include "PybindReferenceItemDefinition.h"
#include "PybindSearchStyle.h"
#include "PybindTag.h"
#include "PybindResource.h"
#include "PybindResourceItem.h"
#include "PybindValueItem.h"
#include "PybindValueItemDefinition.h"
#include "PybindValueItemDefinitionTemplate.h"
#include "PybindValueItemTemplate.h"
#include "PybindVoidItem.h"
#include "PybindVoidItemDefinition.h"

#include "smtk/resource/Manager.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void attributePart1(py::module& attribute)
{
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  py::class_< smtk::attribute::Tag > smtk_attribute_Tag = pybind11_init_smtk_attribute_Tag(attribute);
  py::class_< smtk::attribute::Analyses > smtk_attribute_Analyses = pybind11_init_smtk_attribute_Analyses(attribute);
  pybind11_init_smtk_attribute_SearchStyle(attribute);
  py::class_< smtk::attribute::AttributeCopyOptions > smtk_attribute_AttributeCopyOptions = pybind11_init_smtk_attribute_AttributeCopyOptions(attribute);
  py::class_< smtk::attribute::AttributeAssignmentOptions > smtk_attribute_AttributeAssignmentOptions = pybind11_init_smtk_attribute_AttributeAssignmentOptions(attribute);
  py::class_< smtk::attribute::ItemAssignmentOptions > smtk_attribute_ItemAssignmentOptions = pybind11_init_smtk_attribute_ItemAssignmentOptions(attribute);
  py::class_< smtk::attribute::CopyAssignmentOptions > smtk_attribute_CopyAssignmentOptions = pybind11_init_smtk_attribute_CopyAssignmentOptions(attribute);
  PySharedPtrClass< smtk::attribute::Attribute > smtk_attribute_Attribute = pybind11_init_smtk_attribute_Attribute(attribute);
  PySharedPtrClass< smtk::attribute::Definition > smtk_attribute_Definition = pybind11_init_smtk_attribute_Definition(attribute);
  PySharedPtrClass< smtk::attribute::Item > smtk_attribute_Item = pybind11_init_smtk_attribute_Item(attribute);
  PySharedPtrClass< smtk::attribute::ItemDefinition > smtk_attribute_ItemDefinition = pybind11_init_smtk_attribute_ItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::FileSystemItem, smtk::attribute::Item > smtk_attribute_FileSystemItem = pybind11_init_smtk_attribute_FileSystemItem(attribute);
  PySharedPtrClass< smtk::attribute::FileSystemItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_FileSystemItemDefinition = pybind11_init_smtk_attribute_FileSystemItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::GroupItem, smtk::attribute::Item > smtk_attribute_GroupItem = pybind11_init_smtk_attribute_GroupItem(attribute);
  PySharedPtrClass< smtk::attribute::GroupItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_GroupItemDefinition = pybind11_init_smtk_attribute_GroupItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::ReferenceItem, smtk::attribute::Item > smtk_attribute_ReferenceItem = pybind11_init_smtk_attribute_ReferenceItem(attribute);
  PySharedPtrClass< smtk::attribute::ReferenceItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_ReferenceItemDefinition = pybind11_init_smtk_attribute_ReferenceItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::Resource> smtk_attribute_Resource = pybind11_init_smtk_attribute_Resource(attribute);
  PySharedPtrClass< smtk::attribute::ValueItem, smtk::attribute::Item > smtk_attribute_ValueItem = pybind11_init_smtk_attribute_ValueItem(attribute);
  PySharedPtrClass< smtk::attribute::ValueItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_ValueItemDefinition = pybind11_init_smtk_attribute_ValueItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::VoidItem, smtk::attribute::Item > smtk_attribute_VoidItem = pybind11_init_smtk_attribute_VoidItem(attribute);
  PySharedPtrClass< smtk::attribute::VoidItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_VoidItemDefinition = pybind11_init_smtk_attribute_VoidItemDefinition(attribute);
}
