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
    m_observer = from->observers().insert([this](Task&, State prev, State next) {
      if (prev < next && next >= State::Completable)
      {
        this->reconfigureTask();
      }
    });
  }
}

} // namespace task
} // namespace smtk
