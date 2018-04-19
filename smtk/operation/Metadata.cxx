//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Metadata.h"

#include "smtk/operation/SpecificationOps.h"

#include "smtk/resource/Component.h"

#include "smtk/attribute/Definition.h"

namespace smtk
{
namespace operation
{

Metadata::Metadata(const std::string& typeName, Operation::Index index,
  Operation::Specification specification,
  std::function<std::shared_ptr<smtk::operation::Operation>(void)> createFunctor)
  : create(createFunctor)
  , m_typeName(typeName)
  , m_index(index)
  , m_specification(specification)
  , m_primaryAssociation(nullptr)
{
  // Extract all of the component definitions once, rather than invoking this
  // call every time Metadata::acceptsComponent() is called.
  ComponentDefinitionVector componentDefinitions = extractComponentDefinitions(specification);

  m_acceptsComponent = [=](const smtk::resource::ComponentPtr& component) {
    for (auto& componentDefinition : componentDefinitions)
    {
      if (componentDefinition->isValueValid(component))
      {
        return true;
      }
    }
    return false;
  };

  Operation::Definition opDef = extractParameterDefinition(specification, typeName);
  smtk::attribute::ConstReferenceItemDefinitionPtr rule =
    opDef ? opDef->associationRule() : nullptr;
  if (rule)
  {
    m_primaryAssociation = rule;
  }
}

std::set<std::string> Metadata::groups() const
{
  return extractTagNames(m_specification);
}

} // operation namespace
} // smtk namespace
