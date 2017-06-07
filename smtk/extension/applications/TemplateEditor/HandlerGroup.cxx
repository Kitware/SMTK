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

#include "smtk/attribute/GroupItemDefinition.h"

#include "HandlerGroup.h"
#include "ui_ItemDefGroupForm.h"

HandlerGroup::HandlerGroup()
  : Ui(new Ui::ItemDefGroupForm)
{
}

HandlerGroup::~HandlerGroup() = default;

bool HandlerGroup::initialize_impl(QWidget* parent)
{
  this->Ui->setupUi(parent);
  QObject::connect(this->Ui->cbCommonLabel, SIGNAL(toggled(bool)), this->Ui->leCommonLabel,
    SLOT(setEnabled(bool)));

  if (this->ItemDef)
  {
    const auto item = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(this->ItemDef);

    this->Ui->leNumReqGroups->setText(QString::number(item->numberOfRequiredGroups()));
    this->Ui->leMaxNumGroups->setText(QString::number(item->maxNumberOfGroups()));
    this->Ui->cbExtensible->setChecked(item->isExtensible());

    const bool useCommonLabel = item->usingCommonSubGroupLabel();
    this->Ui->cbCommonLabel->setChecked(useCommonLabel);
    this->Ui->leCommonLabel->setVisible(useCommonLabel);
    this->Ui->leCommonLabel->setText(QString::fromStdString(item->subGroupLabel(0)));
  }
  return true;
}

smtk::attribute::ItemDefinitionPtr HandlerGroup::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(this->ItemDef);

  item->setNumberOfRequiredGroups(static_cast<size_t>(this->Ui->leNumReqGroups->text().toInt()));
  item->setMaxNumberOfGroups(static_cast<size_t>(this->Ui->leMaxNumGroups->text().toInt()));
  item->setIsExtensible(this->Ui->cbExtensible->isChecked());

  if (this->Ui->cbCommonLabel->isChecked())
  {
    item->setCommonSubGroupLabel(this->Ui->leCommonLabel->text().toStdString());
  }

  return this->ItemDef;
}

smtk::attribute::ItemDefinitionPtr HandlerGroup::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::GroupItemDefinition::New(name);
}
