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
#include "PybindComponentItem.h"
#include "PybindComponentItemDefinition.h"
#include "PybindDateTimeItem.h"
#include "PybindDateTimeItemDefinition.h"
#include "PybindDefinition.h"
#include "PybindDoubleItem.h"
#include "PybindDoubleItemDefinition.h"
#include "PybindFileItem.h"
#include "PybindFileItemDefinition.h"
#include "PybindGroupItem.h"
#include "PybindGroupItemDefinition.h"
#include "PybindIntItem.h"
#include "PybindIntItemDefinition.h"
#include "PybindItem.h"
#include "PybindItemDefinition.h"
#include "PybindModelEntityItem.h"
#include "PybindModelEntityItemDefinition.h"
#include "PybindQueries.h"
#include "PybindReferenceItem.h"
#include "PybindReferenceItemDefinition.h"
#include "PybindRegistrar.h"
#include "PybindResourceItem.h"
#include "PybindResourceItemDefinition.h"
#include "PybindSearchStyle.h"
#include "PybindStringItem.h"
#include "PybindStringItemDefinition.h"
#include "PybindTag.h"
#include "PybindResource.h"
#include "PybindValueItem.h"
#include "PybindValueItemDefinition.h"
#include "PybindValueItemDefinitionTemplate.h"
#include "PybindValueItemTemplate.h"
#include "PybindVoidItem.h"
#include "PybindVoidItemDefinition.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

void attributePart3(py::module& attribute)
{
  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::attribute::DateTimeItem, smtk::attribute::Item > smtk_attribute_DateTimeItem = pybind11_init_smtk_attribute_DateTimeItem(attribute);
  PySharedPtrClass< smtk::attribute::DateTimeItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_DateTimeItemDefinition = pybind11_init_smtk_attribute_DateTimeItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::ResourceItem, smtk::attribute::ReferenceItem > smtk_attribute_ResourceItem = pybind11_init_smtk_attribute_ResourceItem(attribute);
  PySharedPtrClass< smtk::attribute::ResourceItemDefinition, smtk::attribute::ReferenceItemDefinition > smtk_attribute_ResourceItemDefinition = pybind11_init_smtk_attribute_ResourceItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::ComponentItem, smtk::attribute::ReferenceItem > smtk_attribute_ComponentItem = pybind11_init_smtk_attribute_ComponentItem(attribute);
  PySharedPtrClass< smtk::attribute::ComponentItemDefinition, smtk::attribute::ItemDefinition > smtk_attribute_ComponentItemDefinition = pybind11_init_smtk_attribute_ComponentItemDefinition(attribute);
  PySharedPtrClass< smtk::attribute::ModelEntityItem, smtk::attribute::ComponentItem > smtk_attribute_ModelEntityItem = pybind11_init_smtk_attribute_ModelEntityItem(attribute);
  PySharedPtrClass< smtk::attribute::ModelEntityItemDefinition, smtk::attribute::ComponentItemDefinition > smtk_attribute_ModelEntityItemDefinition = pybind11_init_smtk_attribute_ModelEntityItemDefinition(attribute);

  py::class_< smtk::attribute::Registrar > smtk_attribute_Registrar = pybind11_init_smtk_attribute_Registrar(attribute);
}
