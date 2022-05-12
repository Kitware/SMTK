//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/NameManager.h"

#include "smtk/model/Entity.h"

#include "smtk/resource/PersistentObject.h"

#include "smtk/common/StringUtil.h"

#include <sstream>

namespace smtk
{
namespace view
{

NameManager::NameManager() = default;

std::string NameManager::nameForObject(const smtk::resource::PersistentObject& obj)
{
  std::string objType;
  const auto* entity = dynamic_cast<const smtk::model::Entity*>(&obj);
  if (entity)
  {
    objType = entity->flagSummary();
  }
  if (objType.empty())
  {
    auto typeName = obj.typeName();
    std::size_t cut = typeName.rfind("::");
    if (cut != std::string::npos)
    {
      objType = typeName.substr(cut + 2);
    }
    else
    {
      objType = typeName;
    }
    objType = smtk::common::StringUtil::lower(objType);
  }

  std::ostringstream ns;
  ns << objType << " " << m_counter++;
  return ns.str();
}

} // namespace view
} // namespace smtk
