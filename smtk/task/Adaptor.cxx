//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Adaptor.h"

#include "smtk/task/json/Configurator.txx"
#include "smtk/task/json/Helper.h"

namespace smtk
{
namespace task
{

Adaptor::Adaptor()
  : m_id(smtk::common::UUID::random())
{
}

Adaptor::Adaptor(const Configuration& config)
{
  this->configureId(config);
}

Adaptor::Adaptor(const Configuration& config, Task* from, Task* to)
  : m_from(from)
  , m_to(to)
{
  this->configureId(config);
  if (from)
  {
    m_observer = from->observers().insert(
      [this](Task&, State prev, State next) { this->updateDownstreamTask(prev, next); });
  }
}

const smtk::resource::ResourcePtr Adaptor::resource() const
{
  if (m_from)
  {
    return m_from->resource();
  }
  if (m_to)
  {
    return m_to->resource();
  }
  return smtk::resource::ResourcePtr();
}

std::string Adaptor::name() const
{
  return this->typeName() + " " + m_id.toString().substr(4);
}

bool Adaptor::setId(const common::UUID& uid)
{
  if (m_id == uid || uid.isNull())
  {
    return false;
  }
  m_id = uid;
  return true;
}

bool Adaptor::reconfigureTask()
{
  // NOTICE! When you remove this method, make updateDownstreamTask
  // pure virtual so that subclasses must provide an implementation.
  return false;
}

bool Adaptor::updateDownstreamTask(State upstreamPrev, State upstreamNext)
{
  if (upstreamPrev < upstreamNext && upstreamNext >= State::Completable)
  {
    return this->reconfigureTask();
  }
  return false;
}

void Adaptor::configureId(const Configuration& config)
{
  auto it = config.find("id");
  if (it != config.end())
  {
    if (it->is_number_integer())
    {
      // taskSwizzle = jTaskId.get<json::Helper::SwizzleId>();
      m_id = smtk::common::UUID::random();
    }
    else if (it->is_string())
    {
      m_id = it->get<smtk::common::UUID>();
    }
  }
  else
  {
    m_id = smtk::common::UUID::random();
  }
}

} // namespace task
} // namespace smtk
