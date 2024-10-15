//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/DisconnectPorts.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/json/jsonProject.h"

#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/operators/DisconnectPorts_xml.h"

#include <string>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{

bool DisconnectPorts::ableToOperate()
{
  auto arcType = this->parameters()->findString("arc type");
  auto endpoints = dynamic_pointer_cast<smtk::attribute::GroupItem>(
    arcType->findChild("port endpoints", smtk::attribute::SearchStyle::IMMEDIATE_ACTIVE));
  std::size_t nn = endpoints->numberOfGroups();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto fromPortItem = endpoints->findAs<smtk::attribute::ReferenceItem>(ii, "from");
    auto toPortItem = endpoints->findAs<smtk::attribute::ComponentItem>(ii, "to");
    if (!fromPortItem->isSet() || !toPortItem->isSet())
    {
      return false;
    }
    auto fromObj = fromPortItem->value();
    auto toPort = toPortItem->valueAs<smtk::task::Port>();
    if (toPort->direction() != smtk::task::Port::Direction::In)
    {
      // The downstream port must be an input port.
      return false;
    }
    // Is there already a connection between fromObj and toPort?
    if (toPort->connections().find(fromObj.get()) == toPort->connections().end())
    {
      return false;
    }
  }
  return this->Superclass::ableToOperate();
}

DisconnectPorts::Result DisconnectPorts::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");
  auto arcType = this->parameters()->findString("arc type");
  auto endpoints = dynamic_pointer_cast<smtk::attribute::GroupItem>(
    arcType->findChild("port endpoints", smtk::attribute::SearchStyle::IMMEDIATE_ACTIVE));
  std::size_t nn = endpoints->numberOfGroups();
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto fromPortItem = endpoints->findAs<smtk::attribute::ReferenceItem>(ii, "from");
    auto toPortItem = endpoints->findAs<smtk::attribute::ComponentItem>(ii, "to");

    auto fromObj = fromPortItem->value();
    auto toPort = toPortItem->valueAs<smtk::task::Port>();
    toPort->connections().erase(fromObj.get());
    modified->appendValue(toPort);
    if (auto fromPort = std::dynamic_pointer_cast<smtk::task::Port>(fromObj))
    {
      fromPort->connections().erase(toPort.get());
      modified->appendValue(fromPort);
    }
  }

  return result;
}

const char* DisconnectPorts::xmlDescription() const
{
  return DisconnectPorts_xml;
}

} // namespace task
} // namespace smtk
