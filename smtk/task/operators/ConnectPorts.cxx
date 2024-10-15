//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/ConnectPorts.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
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

#include "smtk/task/operators/ConnectPorts_xml.h"

#include <string>

using namespace smtk::string::literals;

namespace smtk
{
namespace task
{

bool ConnectPorts::ableToOperate()
{
  auto fromPorts = this->parameters()->associations();
  auto toPorts = this->parameters()->findComponent("to");
  std::size_t nn = fromPorts->numberOfValues();
  if (nn != toPorts->numberOfValues())
  {
    return false;
  }
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    if (!fromPorts->isSet(ii) || !toPorts->isSet(ii))
    {
      return false;
    }
    auto fromObj = fromPorts->value(ii);
    auto toPort = toPorts->valueAs<smtk::task::Port>(ii);
    if (toPort->direction() != smtk::task::Port::Direction::In)
    {
      // The downstream port must be an input port.
      return false;
    }
    // is the toPort already connected to the fromObj?
    if (toPort->connections().find(fromObj.get()) != toPort->connections().end())
    {
      return false;
    }
    auto fromPort = std::dynamic_pointer_cast<smtk::task::Port>(fromObj);
    const auto& toTypes = toPort->dataTypes();
    if (fromPort)
    {
      if (fromPort->direction() != smtk::task::Port::Direction::Out)
      {
        // The upstream port must be an output port.
        return false;
      }
      // Check to make sure the ports are not part of the same task
      if (fromPort->parent() == toPort->parent())
      {
        return false;
      }
      // At least one of the data types produced by the input port should
      // be consumable by the output port.
      const auto& fromTypes = fromPort->dataTypes();
      bool okToConnect = false;
      for (const auto& fromType : fromTypes)
      {
        if (toTypes.find(fromType) != toTypes.end())
        {
          okToConnect = true;
          break;
        }
      }
      if (!okToConnect)
      {
        // None of the information provide by the input port could be used
        // by the output port
        return false;
      }
    }
    else if (fromObj && toTypes.find("smtk::task::ObjectsInRoles"_token) == toTypes.end())
    {
      // The downstream port does not allow "raw" objects to be connected.
      return false;
    }
    else if (!fromObj)
    {
      return false;
    }
  }
  return this->Superclass::ableToOperate();
}

ConnectPorts::Result ConnectPorts::operateInternal()
{
  auto fromPorts = this->parameters()->associations();
  auto toPorts = this->parameters()->findComponent("to");
  std::size_t nn = fromPorts->numberOfValues();

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  auto modified = result->findComponent("modified");

  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    auto fromObj = fromPorts->value(ii);
    auto toPort = toPorts->valueAs<smtk::task::Port>(ii);
    toPort->connections().insert(fromObj.get());
    modified->appendValue(toPort);
    if (auto fromPort = std::dynamic_pointer_cast<smtk::task::Port>(fromObj))
    {
      fromPort->connections().insert(toPort.get());
      modified->appendValue(fromPort);
    }
  }

  return result;
}

const char* ConnectPorts::xmlDescription() const
{
  return ConnectPorts_xml;
}

} // namespace task
} // namespace smtk
