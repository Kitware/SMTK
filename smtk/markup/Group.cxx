//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Group.h"

#include "smtk/markup/Traits.h"

namespace smtk
{
namespace markup
{

Group::~Group() = default;

void Group::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

bool Group::setKeys(const std::weak_ptr<smtk::markup::AssignedIds>& keys)
{
  auto mlocked = m_keys.lock();
  auto vlocked = keys.lock();
  if (mlocked == vlocked)
  {
    return false;
  }
  m_keys = vlocked;
  return true;
}

const std::weak_ptr<smtk::markup::AssignedIds>& Group::keys() const
{
  return m_keys;
}

std::weak_ptr<smtk::markup::AssignedIds>& Group::keys()
{
  return m_keys;
}

ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, OutgoingArc> Group::members() const
{
  return this->outgoing<arcs::GroupsToMembers>();
}

ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, OutgoingArc> Group::members()
{
  return this->outgoing<arcs::GroupsToMembers>();
}

} // namespace markup
} // namespace smtk
