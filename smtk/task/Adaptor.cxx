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

namespace smtk
{
namespace task
{

Adaptor::Adaptor() = default;

Adaptor::Adaptor(const Configuration& config)
{
  (void)config; // subclasses may use this
}

Adaptor::Adaptor(const Configuration& config, Task* from, Task* to)
  : m_from(from)
  , m_to(to)
{
  (void)config; // subclasses may use this
  if (from)
  {
    m_observer = from->observers().insert(
      [this](Task&, State prev, State next) { this->updateDownstreamTask(prev, next); });
  }
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

} // namespace task
} // namespace smtk
