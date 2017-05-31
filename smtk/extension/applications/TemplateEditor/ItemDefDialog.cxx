//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QDialogButtonBox>

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"

#include "ItemDefDialog.h"
#include "ItemDefinitionHelper.h"
#include "ui_ItemDefDialog.h"

// ------------------------------------------------------------------------
ItemDefDialog::ItemDefDialog(QWidget* parent)
  : InputDialog(parent)
  , Ui(new Ui::ItemDefDialog)
{
  this->setWindowTitle(tr("Item Definition"));
  this->Ui->setupUi(this->centralWidget());
  this->Ui->cbTypes->addItems(ItemDefinitionHelper::getTypes());
  this->setEditMode(EditMode::NEW);

  connect(this->Ui->leName, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));
}

// ------------------------------------------------------------------------
ItemDefDialog::~ItemDefDialog() = default;

// ------------------------------------------------------------------------
void ItemDefDialog::setItemDef(smtk::attribute::ItemDefinitionPtr def)
{
  if (!def)
  {
    std::cerr << "ERROR: invalid Item Definition!\n";
    return;
  }

  using SMTKItem = smtk::attribute::Item;

  this->ItemDef = def;
  const SMTKItem::Type type = def->type();
  this->Ui->cbTypes->setCurrentIndex(type);
  this->Ui->leName->setText(QString::fromStdString(def->name()));
  this->Ui->leLabel->setText(QString::fromStdString(def->label()));
  this->Ui->leVersion->setText(QString::number(def->version()));
  //this->Ui->leAdvanceLevel->setText(QString::number(def->advanceLevel()));

  if (type == SMTKItem::VOID)
  {
    this->Ui->gbConcreteParams->hide();
  }
  else
  {
    const QString title = QString::fromStdString(SMTKItem::type2String(type)) + " Properties";
    this->Ui->gbConcreteParams->setTitle(title);
  }
}

// ------------------------------------------------------------------------
void ItemDefDialog::setAttDef(smtk::attribute::DefinitionPtr def)
{
  if (!def)
  {
    std::cerr << "ERROR: invalid Attribute Definition!\n";
    return;
  }

  this->AttDef = def;
}

// ------------------------------------------------------------------------
const ItemDefProperties& ItemDefDialog::getInputValues()
{
  //auto props = this->Customizer->getInputValues();
  this->Properties.Type = this->Ui->cbTypes->currentText().toStdString();
  this->Properties.Name = this->Ui->leName->text().toStdString();
  this->Properties.Label = this->Ui->leLabel->text().toStdString();
  ///TODO Add validator to int line-edits.
  this->Properties.Version = this->Ui->leVersion->text().toInt();

  return this->Properties;
}

// ------------------------------------------------------------------------
bool ItemDefDialog::validate_impl()
{
  bool valid = true;

  const QString name = this->Ui->leName->text();
  valid &= !name.isEmpty();

  if (this->AttDef)
  {
    int pos = this->AttDef->findItemPosition(name.toStdString());
    valid &= pos < 0;

    /// TODO Validate GroupItemDefinition entries too.
  }

  return valid;
}

// ------------------------------------------------------------------------
void ItemDefDialog::setEditMode(EditMode mode)
{
  bool enable = true;
  using QDBB = QDialogButtonBox;
  QDBB::StandardButtons buttons = QDBB::Cancel | QDBB::Ok;
  switch (mode)
  {
    case EditMode::NEW:
      this->Ui->cbTypes->setEnabled(enable);
      this->Ui->leName->setEnabled(enable);
      break;
    case EditMode::EDIT:
      this->Ui->cbTypes->setEnabled(false);
      this->Ui->leName->setEnabled(false);
      break;
    case EditMode::SHOW:
      this->Ui->cbTypes->setEnabled(false);
      this->Ui->leName->setEnabled(false);
      enable = false;
      buttons = QDBB::Close;
      break;
    default:
      std::cout << "Error: Invalid edit mode!\n";
  }

  this->Ui->leLabel->setEnabled(enable);
  this->Ui->leVersion->setEnabled(enable);
  this->Ui->gbConcreteParams->setEnabled(enable);
  this->buttonBox()->setStandardButtons(buttons);
}
