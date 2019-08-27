//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <iostream>

#include <QGroupBox>

#include <smtk/attribute/Item.h>
#include <smtk/attribute/ItemDefinition.h>

#include "HandlerGroup.h"
#include "HandlerItemDef.h"
#include "HandlerValue.h"
#include "HandlerVoid.h"

// -----------------------------------------------------------------------------
HandlerItemDef::HandlerItemDef() = default;

// -----------------------------------------------------------------------------
HandlerItemDef::~HandlerItemDef() = default;

// -----------------------------------------------------------------------------
std::shared_ptr<HandlerItemDef> HandlerItemDef::create(const int type)
{
  using namespace smtk::attribute;
  switch (type)
  {
    case Item::DoubleType:
      return std::make_shared<HandlerDouble>();
    case Item::GroupType:
      return std::make_shared<HandlerGroup>();
    case Item::IntType:
      return std::make_shared<HandlerInt>();
    case Item::StringType:
      return std::make_shared<HandlerString>();
    case Item::ColorType:
    case Item::VoidType:
      return std::make_shared<HandlerVoid>();
    case Item::FileType:
      return std::make_shared<HandlerFile>();
    case Item::DirectoryType:
      return std::make_shared<HandlerDirectory>();
    case Item::DateTimeType:
      return std::make_shared<HandlerDateTime>();
    case Item::ComponentType:
      return std::make_shared<HandlerComponent>();
    case Item::ResourceType:
      return std::make_shared<HandlerResource>();
    case Item::ModelEntityType:
    default:
      std::cerr << "Error: Unknown type!\n";
      return nullptr;
  }
}

// -----------------------------------------------------------------------------
bool HandlerItemDef::initialize(ItemDefPtr itemDef, QWidget* parentWidget)
{
  if (itemDef)
  {
    using SMTKItem = smtk::attribute::Item;
    const QString title =
      QString::fromStdString(SMTKItem::type2String(itemDef->type())) + " Properties";

    auto groupBox = qobject_cast<QGroupBox*>(parentWidget);
    groupBox->setTitle(title);
  }

  this->ItemDef = itemDef;
  return this->initialize_impl(parentWidget);
}

// -----------------------------------------------------------------------------
HandlerItemDef::ItemDefPtr HandlerItemDef::updateItemDef(const std::string& name)
{
  if (!this->ItemDef)
  {
    this->ItemDef = this->createItemDef(name);
  }

  return this->updateItemDef_impl();
}

// -----------------------------------------------------------------------------
HandlerItemDef::ItemDefPtr HandlerItemDef::createItemDef(const std::string& name)
{
  return this->createItemDef_impl(name);
}
