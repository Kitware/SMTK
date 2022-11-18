//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Field.h"

#include "smtk/markup/Traits.h"

namespace smtk
{
namespace markup
{

Field::~Field() = default;

void Field::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  this->Superclass::initialize(data, helper);
  m_fieldType = smtk::string::Token::fromHash(data["field_type"].get<smtk::string::Hash>());
}

void Field::initialize(const std::string& name, smtk::string::Token fieldType)
{
  m_fieldType = fieldType;
  // Normally we would call this->setName(name), but initialize() is called
  // before the node is added to the resource, so this will fail.
  m_name = name;
}

ArcEndpointInterface<arcs::FieldsToShapes, ConstArc, OutgoingArc> Field::shapes() const
{
  return this->outgoing<arcs::FieldsToShapes>();
}

ArcEndpointInterface<arcs::FieldsToShapes, NonConstArc, OutgoingArc> Field::shapes()
{
  return this->outgoing<arcs::FieldsToShapes>();
}

} // namespace markup
} // namespace smtk
