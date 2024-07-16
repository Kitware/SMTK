//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

#include "AttDefDialog.h"
#include "ui_AttDefDialog.h"

// ------------------------------------------------------------------------
AttDefDialog::AttDefDialog(QWidget* parent)
  : InputDialog(parent)
  , Ui(new Ui::AttDefDialog)
{
  this->setWindowTitle(tr("New Attribute Definition"));
  this->Ui->setupUi(this->centralWidget());

  connect(
    this->Ui->cbBaseType, SIGNAL(toggled(bool)), this->Ui->laBaseType, SLOT(setEnabled(bool)));
  connect(this->Ui->leType, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));
}

// ------------------------------------------------------------------------
AttDefDialog::~AttDefDialog() = default;

// ------------------------------------------------------------------------
void AttDefDialog::setBaseAttDef(smtk::attribute::DefinitionPtr def)
{
  this->BaseDef = def;
  this->Ui->laBaseType->setText(QString::fromStdString(def->type()));
}

// ------------------------------------------------------------------------
const AttDefContainer& AttDefDialog::getInputValues()
{
  if (this->Ui->cbBaseType->isChecked())
  {
    this->Container.BaseType = this->Ui->laBaseType->text().toStdString();
  }

  this->Container.Type = this->Ui->leType->text().toStdString();
  this->Container.IsUnique = this->Ui->cbUnique->isChecked();
  this->Container.IsAbstract = this->Ui->cbAbstract->isChecked();
  this->Container.Label = this->Ui->leLabel->text().toStdString();

  return this->Container;
}

// ------------------------------------------------------------------------
bool AttDefDialog::validate_impl()
{
  bool valid = true;

  const QString type = this->Ui->leType->text();
  valid &= !type.isEmpty();

  auto def = this->BaseDef->attributeResource()->findDefinition(type.toStdString());
  valid &= !def;

  return valid;
}
