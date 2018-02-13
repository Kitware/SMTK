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

namespace smtk
{
namespace operation
{

Metadata::Metadata(const std::string& uniqueName, Operation::Index index,
  Operation::Specification specification,
  std::function<std::shared_ptr<smtk::operation::Operation>(void)> createFunctor)
  : create(createFunctor)
  , m_uniqueName(uniqueName)
  , m_index(index)
  , m_specification(specification)
{
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
}

} // operation namespace
} // smtk namespace
