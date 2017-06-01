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

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "HandlerValue.h"
#include "ui_ItemDefDoubleForm.h"
#include "ui_ItemDefIntForm.h"
#include "ui_ItemDefStringForm.h"
#include "ui_ItemDefValueForm.h"

HandlerValue::HandlerValue()
  : Ui(new Ui::ItemDefValueForm)
{
}

HandlerValue::~HandlerValue() = default;

bool HandlerValue::initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent)
{
  this->Ui->setupUi(parent);

  if (def)
  {
    auto item = std::static_pointer_cast<smtk::attribute::ValueItemDefinition>(this->ItemDef);
    /// TODO particular properties for value (populate widgets).
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerValue::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::ValueItemDefinition>(this->ItemDef);

  /// TODO particular properties for Value (set to Itemdef).
  return this->ItemDef;
}

QWidget* HandlerValue::getSubclassWidget()
{
  return this->Ui->wSubclass;
}

///////////////////////////////////////////////////////////////////////////
HandlerString::HandlerString()
  : Ui(new Ui::ItemDefStringForm)
{
}

HandlerString::~HandlerString() = default;

bool HandlerString::initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent)
{
  HandlerValue::initialize_impl(def, parent);

  this->ItemDef = def;
  this->Ui->setupUi(HandlerValue::getSubclassWidget());

  if (def)
  {
    auto item = std::static_pointer_cast<smtk::attribute::StringItemDefinition>(this->ItemDef);
    this->Ui->cbSecure->setChecked(item->isSecure());
    this->Ui->cbMultiline->setChecked(item->isMultiline());
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerString::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::StringItemDefinition>(this->ItemDef);

  item->setIsSecure(this->Ui->cbSecure->isChecked());
  item->setIsMultiline(this->Ui->cbMultiline->isChecked());

  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerString::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::StringItemDefinition::New(name);
}

///////////////////////////////////////////////////////////////////////////
HandlerDouble::HandlerDouble()
  : Ui(new Ui::ItemDefDoubleForm)
{
}

HandlerDouble::~HandlerDouble() = default;

bool HandlerDouble::initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent)
{
  HandlerValue::initialize_impl(def, parent);

  this->ItemDef = def;
  this->Ui->setupUi(HandlerValue::getSubclassWidget());

  if (def)
  {
    auto item = std::static_pointer_cast<smtk::attribute::DoubleItemDefinition>(this->ItemDef);
    ///TODO Specifics double
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerDouble::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::DoubleItemDefinition>(this->ItemDef);

  ///TODO Specifics double
  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerDouble::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DoubleItemDefinition::New(name);
}

///////////////////////////////////////////////////////////////////////////
HandlerInt::HandlerInt()
  : Ui(new Ui::ItemDefIntForm)
{
}

HandlerInt::~HandlerInt() = default;

bool HandlerInt::initialize_impl(smtk::attribute::ItemDefinitionPtr def, QWidget* parent)
{
  HandlerValue::initialize_impl(def, parent);

  this->ItemDef = def;
  this->Ui->setupUi(HandlerValue::getSubclassWidget());

  if (def)
  {
    auto item = std::static_pointer_cast<smtk::attribute::IntItemDefinition>(this->ItemDef);
    ///TODO Specifics int
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerInt::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::IntItemDefinition>(this->ItemDef);

  ///TODO Specifics int
  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerInt::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::IntItemDefinition::New(name);
}
