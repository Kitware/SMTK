//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QWidget>

#include "smtk/attribute/VoidItemDefinition.h"

#include "HandlerVoid.h"

// -----------------------------------------------------------------------------
HandlerVoid::HandlerVoid()
{
}

// -----------------------------------------------------------------------------
HandlerVoid::~HandlerVoid() = default;

// -----------------------------------------------------------------------------
bool HandlerVoid::initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerVoid::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerVoid::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::VoidItemDefinition::New(name);
}
