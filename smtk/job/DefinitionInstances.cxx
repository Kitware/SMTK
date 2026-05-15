//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/DefinitionInstances.h"

namespace smtk
{
namespace job
{

std::shared_ptr<Definition> DefinitionInstances::findByName(const std::string& name)
{
  Definition::Ptr result = nullptr;
  this->visit([&result, name](const std::shared_ptr<Definition>& definition) {
    if (definition->name() == name)
    {
      result = definition;
      return smtk::common::Visit::Halt;
    }
    return smtk::common::Visit::Continue;
  });
  return result;
}

} // namespace job
} // namespace smtk
