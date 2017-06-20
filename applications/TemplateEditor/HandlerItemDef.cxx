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
    case Item::DOUBLE:
      return std::make_shared<HandlerDouble>();
    case Item::GROUP:
      return std::make_shared<HandlerGroup>();
    case Item::INT:
      return std::make_shared<HandlerInt>();
    case Item::STRING:
      return std::make_shared<HandlerString>();
    case Item::COLOR:
    case Item::VOID:
      return std::make_shared<HandlerVoid>();
    case Item::FILE:
      return std::make_shared<HandlerFile>();
    case Item::DIRECTORY:
      return std::make_shared<HandlerDirectory>();
    case Item::MODEL_ENTITY:
      return std::make_shared<HandlerModelEntity>();
    case Item::MESH_SELECTION:
      return std::make_shared<HandlerMeshSelection>();
    case Item::MESH_ENTITY:
      return std::make_shared<HandlerMeshEntity>();
    case Item::DATE_TIME:
      return std::make_shared<HandlerDateTime>();
    case Item::ATTRIBUTE_REF:
      return std::make_shared<HandlerRef>();
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
