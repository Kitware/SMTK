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

#include "PybindAttribute.h"
#include "PybindDateTimeItem.h"
#include "PybindDateTimeItemDefinition.h"
#include "PybindDefinition.h"
#include "PybindDirectoryItem.h"
#include "PybindDirectoryItemDefinition.h"
#include "PybindDoubleItem.h"
#include "PybindDoubleItemDefinition.h"
#include "PybindFileItem.h"
#include "PybindFileItemDefinition.h"
#include "PybindFileSystemItem.h"
#include "PybindFileSystemItemDefinition.h"
#include "PybindGroupItem.h"
#include "PybindGroupItemDefinition.h"
#include "PybindIntItem.h"
#include "PybindIntItemDefinition.h"
#include "PybindItem.h"
#include "PybindItemDefinition.h"
#include "PybindMeshItem.h"
#include "PybindMeshItemDefinition.h"
#include "PybindMeshSelectionItem.h"
#include "PybindMeshSelectionItemDefinition.h"
#include "PybindModelEntityItem.h"
#include "PybindModelEntityItemDefinition.h"
#include "PybindRefItem.h"
#include "PybindRefItemDefinition.h"
#include "PybindSearchStyle.h"
#include "PybindStringItem.h"
#include "PybindStringItemDefinition.h"
#include "PybindSystem.h"
#include "PybindValueItem.h"
#include "PybindValueItemDefinition.h"
#include "PybindValueItemDefinitionTemplate.h"
#include "PybindValueItemTemplate.h"
#include "PybindVoidItem.h"
#include "PybindVoidItemDefinition.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindAttribute)
{
  py::module attribute("_smtkPybindAttribute", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  pybind11_init_smtk_attribute_SearchStyle(attribute);
  PySharedPtrClass< smtk::attribute::Attribute > smtk_attribute_Attribute = pybind11_init_smtk_attribute_Attribute(attribute);
  PySharedPtrClass< smtk::attribute::Definition > smtk_attribute_Definition = pybind11_init_smtk_attribute_Definition(attribute);
  PySharedPtrClass< smtk::attribute::Item > smtk_attribute_Item = pybind11_init_smtk_attribute_Item(attribute);
  PySharedPtrClass< smtk::attribute::ItemDefinition > smtk_attribute_ItemDefinition = pybind11_init_smtk_attribute_ItemDefinition(attribute);
  pybind11_init_smtk_attribute_MeshModifyMode(attribute);
  PySharedPtrClass< smtk::attribute::FileSystemItem, smtk::attribute::Item > smtk_attribute_FileSystemItem = pybind11_init_smtk_attribute_FileSystemItem(attribute);
  PySharedPtrClass< smtk::attribute::FileSystemItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_FileSystemItemDefinition = pybind11_init_smtk_attribute_FileSystemItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::GroupItem, smtk::attribute::Item > smtk_attribute_GroupItem = pybind11_init_smtk_attribute_GroupItem(attribute);
  PySharedPtrClass< smtk::attribute::GroupItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_GroupItemDefinition = pybind11_init_smtk_attribute_GroupItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::MeshItem, smtk::attribute::Item > smtk_attribute_MeshItem = pybind11_init_smtk_attribute_MeshItem(attribute);
  PySharedPtrClass< smtk::attribute::MeshItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_MeshItemDefinition = pybind11_init_smtk_attribute_MeshItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::MeshSelectionItem, smtk::attribute::Item > smtk_attribute_MeshSelectionItem = pybind11_init_smtk_attribute_MeshSelectionItem(attribute);
  PySharedPtrClass< smtk::attribute::MeshSelectionItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_MeshSelectionItemDefinition = pybind11_init_smtk_attribute_MeshSelectionItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::ModelEntityItem, smtk::attribute::Item > smtk_attribute_ModelEntityItem = pybind11_init_smtk_attribute_ModelEntityItem(attribute);
  PySharedPtrClass< smtk::attribute::ModelEntityItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_ModelEntityItemDefinition = pybind11_init_smtk_attribute_ModelEntityItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::RefItem, smtk::attribute::Item > smtk_attribute_RefItem = pybind11_init_smtk_attribute_RefItem(attribute);
  PySharedPtrClass< smtk::attribute::RefItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_RefItemDefinition = pybind11_init_smtk_attribute_RefItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::System, smtk::common::Resource > smtk_attribute_System = pybind11_init_smtk_attribute_System(attribute);
  PySharedPtrClass< smtk::attribute::ValueItem, smtk::attribute::Item > smtk_attribute_ValueItem = pybind11_init_smtk_attribute_ValueItem(attribute);
  PySharedPtrClass< smtk::attribute::ValueItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_ValueItemDefinition = pybind11_init_smtk_attribute_ValueItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::VoidItem, smtk::attribute::Item > smtk_attribute_VoidItem = pybind11_init_smtk_attribute_VoidItem(attribute);
  PySharedPtrClass< smtk::attribute::VoidItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_VoidItemDefinition = pybind11_init_smtk_attribute_VoidItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::DirectoryItem, smtk::attribute::FileSystemItem > smtk_attribute_DirectoryItem = pybind11_init_smtk_attribute_DirectoryItem(attribute);
  PySharedPtrClass< smtk::attribute::DirectoryItemDefinition, smtk::attribute::FileSystemItemDefinition > smtk_attribute_DirectoryItemDefinition = pybind11_init_smtk_attribute_DirectoryItemDefinition(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemTemplate<double>, smtk::attribute::ValueItem > smtk_attribute_ValueItemTemplate_double_ = pybind11_init_smtk_attribute_ValueItemTemplate_double_(attribute);
  PySharedPtrClass< smtk::attribute::DoubleItem, smtk::attribute::ValueItemTemplate<double> > smtk_attribute_DoubleItem = pybind11_init_smtk_attribute_DoubleItem(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<double>, smtk::attribute::ValueItemDefinition > smtk_attribute_ValueItemDefinitionTemplate_double_ = pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_double_(attribute);
  PySharedPtrClass< smtk::attribute::DoubleItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<double> > smtk_attribute_DoubleItemDefinition = pybind11_init_smtk_attribute_DoubleItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::FileItem, smtk::attribute::FileSystemItem > smtk_attribute_FileItem = pybind11_init_smtk_attribute_FileItem(attribute);
  PySharedPtrClass< smtk::attribute::FileItemDefinition, smtk::attribute::FileSystemItemDefinition > smtk_attribute_FileItemDefinition = pybind11_init_smtk_attribute_FileItemDefinition(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemTemplate<int>, smtk::attribute::ValueItem > smtk_attribute_ValueItemTemplate_int_ = pybind11_init_smtk_attribute_ValueItemTemplate_int_(attribute);
  PySharedPtrClass< smtk::attribute::IntItem, smtk::attribute::ValueItemTemplate<int> > smtk_attribute_IntItem = pybind11_init_smtk_attribute_IntItem(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<int>, smtk::attribute::ValueItemDefinition > smtk_attribute_ValueItemDefinitionTemplate_int_ = pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_int_(attribute);
  PySharedPtrClass< smtk::attribute::IntItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<int> > smtk_attribute_IntItemDefinition = pybind11_init_smtk_attribute_IntItemDefinition(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemTemplate<std::basic_string<char> >, smtk::attribute::ValueItem > smtk_attribute_ValueItemTemplate_string_ = pybind11_init_smtk_attribute_ValueItemTemplate_string_(attribute);
  PySharedPtrClass< smtk::attribute::StringItem, smtk::attribute::ValueItemTemplate<std::basic_string<char> > > smtk_attribute_StringItem = pybind11_init_smtk_attribute_StringItem(attribute);
  PySharedPtrClass<smtk::attribute::ValueItemDefinitionTemplate<std::basic_string<char> >, smtk::attribute::ValueItemDefinition > smtk_attribute_ValueItemDefinitionTemplate_string_ = pybind11_init_smtk_attribute_ValueItemDefinitionTemplate_string_(attribute);
  PySharedPtrClass< smtk::attribute::StringItemDefinition, smtk::attribute::ValueItemDefinitionTemplate<std::basic_string<char> > > smtk_attribute_StringItemDefinition = pybind11_init_smtk_attribute_StringItemDefinition(attribute);

  PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::Item > smtk_attribute_DateTimeItem = pybind11_init_smtk_attribute_DateTimeItem(attribute);
  PySharedPtrClass< smtk::attribute::DateTimeItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_DateTimeItemDefinition = pybind11_init_smtk_attribute_DateTimeItemDefinition(attribute);

  return attribute.ptr();
}
