//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Label.h"

#include "smtk/markup/Traits.h"

namespace smtk
{
namespace markup
{

Label::~Label() = default;

void Label::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

ArcEndpointInterface<arcs::LabelsToSubjects, ConstArc, OutgoingArc> Label::subjects() const
{
  return this->outgoing<arcs::LabelsToSubjects>();
}

ArcEndpointInterface<arcs::LabelsToSubjects, NonConstArc, OutgoingArc> Label::subjects()
{
  return this->outgoing<arcs::LabelsToSubjects>();
}

} // namespace markup
} // namespace smtk
