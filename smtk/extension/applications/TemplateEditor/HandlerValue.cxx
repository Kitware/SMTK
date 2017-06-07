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

bool HandlerValue::initialize_impl(QWidget* parent)
{
  this->Ui->setupUi(parent);
  QObject::connect(this->Ui->cbCommonLabel, SIGNAL(toggled(bool)), this->Ui->leCommonLabel,
    SLOT(setEnabled(bool)));

  if (this->ItemDef)
  {
    const auto item = std::static_pointer_cast<smtk::attribute::ValueItemDefinition>(this->ItemDef);

    this->Ui->leMaxNumValues->setText(QString::number(item->maxNumberOfValues()));
    this->Ui->leNumReqValues->setText(QString::number(item->numberOfRequiredValues()));
    this->Ui->leUnits->setText(QString::fromStdString(item->units()));
    this->Ui->cbExtensible->setChecked(item->isExtensible());

    const bool useCommonLabel = item->usingCommonLabel();
    this->Ui->cbCommonLabel->setChecked(useCommonLabel);
    this->Ui->leCommonLabel->setEnabled(useCommonLabel);
    this->Ui->leCommonLabel->setText(QString::fromStdString(item->valueLabel(0)));
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerValue::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::ValueItemDefinition>(this->ItemDef);

  item->setMaxNumberOfValues(this->Ui->leMaxNumValues->text().toInt());
  item->setNumberOfRequiredValues(this->Ui->leNumReqValues->text().toInt());
  item->setUnits(this->Ui->leUnits->text().toStdString());
  item->setIsExtensible(this->Ui->cbExtensible->isChecked());

  if (this->Ui->cbCommonLabel->isChecked())
  {
    item->setCommonValueLabel(this->Ui->leCommonLabel->text().toStdString());
  }

  return this->ItemDef;
}

///////////////////////////////////////////////////////////////////////////
HandlerString::HandlerString()
  : Ui(new Ui::ItemDefStringForm)
{
}

HandlerString::~HandlerString() = default;

bool HandlerString::initialize_impl(QWidget* parent)
{
  HandlerValue::initialize_impl(parent);
  auto uiValue = HandlerValue::Ui;
  this->Ui->setupUi(uiValue->wSubclass);

  if (this->ItemDef)
  {
    const auto item =
      std::static_pointer_cast<smtk::attribute::StringItemDefinition>(this->ItemDef);

    //ValueTemplate
    ///TODO differences in QString::fromStdString/QString::number signatures
    ///made it difficult to place this in the base as a template method. Review
    ///architecture (perhaps using MV components here could be better; e.g.
    ///SpreadsheetView, etc.)
    uiValue->leDefaultValue->setText(QString::fromStdString(item->defaultValue()));
    if (item->hasMinRange())
    {
      uiValue->leMinValue->setText(QString::fromStdString(item->minRange()));
      uiValue->cbMinInclusive->setChecked(item->minRangeInclusive());
    }

    if (item->hasMaxRange())
    {
      uiValue->leMaxValue->setText(QString::fromStdString(item->maxRange()));
      uiValue->cbMaxInclusive->setChecked(item->maxRangeInclusive());
    }
    ///TODO discreteValue vector, label/value, (both here and in updateItemDef_impl).
    ///This would need to be changed int every concrete HandlerValue class.

    //StringItemDef
    this->Ui->cbSecure->setChecked(item->isSecure());
    this->Ui->cbMultiline->setChecked(item->isMultiline());
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerString::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::StringItemDefinition>(this->ItemDef);
  auto uiValue = HandlerValue::Ui;

  //ValueTemplate
  item->setDefaultValue(uiValue->leDefaultValue->text().toStdString());
  if (!uiValue->leMinValue->text().isEmpty())
  {
    item->setMinRange(
      uiValue->leMinValue->text().toStdString(), uiValue->cbMinInclusive->isChecked());
  }

  if (!uiValue->leMaxValue->text().isEmpty())
  {
    item->setMaxRange(
      uiValue->leMaxValue->text().toStdString(), uiValue->cbMaxInclusive->isChecked());
  }

  //StringItemDef
  item->setIsSecure(this->Ui->cbSecure->isChecked());
  item->setIsMultiline(this->Ui->cbMultiline->isChecked());

  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerString::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::StringItemDefinition::New(name);
}

///////////////////////////////////////////////////////////////////////////
bool HandlerDouble::initialize_impl(QWidget* parent)
{
  HandlerValue::initialize_impl(parent);
  auto uiValue = HandlerValue::Ui;

  if (this->ItemDef)
  {
    const auto item =
      std::static_pointer_cast<smtk::attribute::DoubleItemDefinition>(this->ItemDef);

    //ValueTemplate
    uiValue->leDefaultValue->setText(QString::number(item->defaultValue()));
    if (item->hasMinRange())
    {
      uiValue->leMinValue->setText(QString::number(item->minRange()));
      uiValue->cbMinInclusive->setChecked(item->minRangeInclusive());
    }

    if (item->hasMaxRange())
    {
      uiValue->leMaxValue->setText(QString::number(item->maxRange()));
      uiValue->cbMaxInclusive->setChecked(item->maxRangeInclusive());
    }
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerDouble::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::DoubleItemDefinition>(this->ItemDef);
  auto uiValue = HandlerValue::Ui;

  //ValueTemplate
  item->setDefaultValue(uiValue->leDefaultValue->text().toDouble());
  if (!uiValue->leMinValue->text().isEmpty())
  {
    item->setMinRange(uiValue->leMinValue->text().toDouble(), uiValue->cbMinInclusive->isChecked());
  }

  if (!uiValue->leMaxValue->text().isEmpty())
  {
    item->setMaxRange(uiValue->leMaxValue->text().toDouble(), uiValue->cbMaxInclusive->isChecked());
  }

  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerDouble::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DoubleItemDefinition::New(name);
}

///////////////////////////////////////////////////////////////////////////
bool HandlerInt::initialize_impl(QWidget* parent)
{
  HandlerValue::initialize_impl(parent);
  auto uiValue = HandlerValue::Ui;

  if (this->ItemDef)
  {
    const auto item = std::static_pointer_cast<smtk::attribute::IntItemDefinition>(this->ItemDef);

    //ValueTemplate
    uiValue->leDefaultValue->setText(QString::number(item->defaultValue()));
    if (item->hasMinRange())
    {
      uiValue->leMinValue->setText(QString::number(item->minRange()));
      uiValue->cbMinInclusive->setChecked(item->minRangeInclusive());
    }

    if (item->hasMaxRange())
    {
      uiValue->leMaxValue->setText(QString::number(item->maxRange()));
      uiValue->cbMaxInclusive->setChecked(item->maxRangeInclusive());
    }
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerInt::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::IntItemDefinition>(this->ItemDef);
  auto uiValue = HandlerValue::Ui;

  //ValueTemplate
  item->setDefaultValue(uiValue->leDefaultValue->text().toInt());
  if (!uiValue->leMinValue->text().isEmpty())
  {
    item->setMinRange(uiValue->leMinValue->text().toInt(), uiValue->cbMinInclusive->isChecked());
  }

  if (!uiValue->leMaxValue->text().isEmpty())
  {
    item->setMaxRange(uiValue->leMaxValue->text().toInt(), uiValue->cbMaxInclusive->isChecked());
  }

  return HandlerValue::updateItemDef_impl();
}

smtk::attribute::ItemDefinitionPtr HandlerInt::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::IntItemDefinition::New(name);
}
