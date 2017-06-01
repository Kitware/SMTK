//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QGroupBox>

#include <smtk/attribute/Item.h>
#include <smtk/attribute/ItemDefinition.h>

#include "HandlerItemDef.h"

HandlerItemDef::HandlerItemDef() = default;

HandlerItemDef::~HandlerItemDef() = default;

bool HandlerItemDef::initialize(smtk::attribute::ItemDefinitionPtr itemDef, QWidget* parentWidget)
{
  if (itemDef)
  {
    using SMTKItem = smtk::attribute::Item;
    const QString title =
      QString::fromStdString(SMTKItem::type2String(itemDef->type())) + " Properties";

    auto groupBox = qobject_cast<QGroupBox*>(parentWidget);
    groupBox->setTitle(title);
  }

  return this->initialize_impl(itemDef, parentWidget);
}

smtk::attribute::ItemDefinitionPtr HandlerItemDef::updateItemDef(const std::string& name)
{
  if (!this->ItemDef)
  {
    this->ItemDef = this->createItemDef(name);
  }

  return this->updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerItemDef::createItemDef(const std::string& name)
{
  return this->createItemDef_impl(name);
}
