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
#include "PybindDirectoryItem.h"
#include "PybindDirectoryItemDefinition.h"
#include "PybindDoubleItem.h"
#include "PybindDoubleItemDefinition.h"
#include "PybindFileItem.h"
#include "PybindFileItemDefinition.h"
#include "PybindIntItem.h"
#include "PybindIntItemDefinition.h"
#include "PybindQueries.h"
#include "PybindStringItem.h"
#include "PybindStringItemDefinition.h"
#include "PybindTag.h"
#include "PybindResource.h"
#include "PybindResourceItem.h"
#include "PybindValueItem.h"
#include "PybindValueItemDefinition.h"
#include "PybindValueItemDefinitionTemplate.h"
#include "PybindValueItemTemplate.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/resource/Manager.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void attributePart2(py::module& attribute)
{
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
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
}
