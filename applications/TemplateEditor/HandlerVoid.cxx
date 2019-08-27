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

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "HandlerVoid.h"
#include "ui_ItemDefRefForm.h"

// -----------------------------------------------------------------------------
bool HandlerVoid::initialize_impl(QWidget* parent)
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

////////////////////////////////////////////////////////////////////////////////
bool HandlerFile::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerFile::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerFile::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::FileItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerDirectory::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDirectory::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDirectory::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DirectoryItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerComponent::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerComponent::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerComponent::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::ComponentItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerResource::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerResource::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerResource::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::ResourceItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerDateTime::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDateTime::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDateTime::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DateTimeItemDefinition::New(name);
}
