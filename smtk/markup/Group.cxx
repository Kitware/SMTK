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
