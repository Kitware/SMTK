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
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"

#include "HandlerItemDef.h"
#include "ItemDefDialog.h"
#include "ui_ItemDefDialog.h"

// ------------------------------------------------------------------------
ItemDefDialog::ItemDefDialog(QWidget* parent)
  : InputDialog(parent)
  , Ui(new Ui::ItemDefDialog)
{
  this->setWindowTitle(tr("Item Definition"));
  this->Ui->setupUi(this->centralWidget());
  this->Ui->cbTypes->addItems(this->getTypeList());

  connect(this->Ui->leName, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));

  validate();
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
  const SMTKItem::Type type = def->type();
  this->Ui->cbTypes->setCurrentIndex(type);
  this->Ui->leName->setText(QString::fromStdString(def->name()));
  this->Ui->leLabel->setText(QString::fromStdString(def->label()));
  this->Ui->leVersion->setText(QString::number(def->version()));
  //this->Ui->leAdvanceLevel->setText(QString::number(def->advanceLevel()));

  this->Handler = HandlerItemDef::create(type);
  this->Handler->initialize(def, this->Ui->gbConcrete);
}

// ------------------------------------------------------------------------
void ItemDefDialog::setValidationInstances(
  smtk::attribute::ItemDefinitionPtr itemDef, smtk::attribute::DefinitionPtr def)
{
  if (itemDef && itemDef->type() == smtk::attribute::Item::GROUP)
  {
    this->ParentGroup = std::static_pointer_cast<smtk::attribute::GroupItemDefinition>(itemDef);
  }
  else
  {
    this->AttDef = def;
  }
}

// ------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr ItemDefDialog::getItemDef()
{
  // Update ItemDefinition with input values
  auto itemDef = this->Handler->updateItemDef(this->Ui->leName->text().toStdString());

  ///TODO Add numeric validator to int line-edits.
  itemDef->setLabel(this->Ui->leLabel->text().toStdString());
  itemDef->setVersion(this->Ui->leVersion->text().toInt());

  return itemDef;
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
  }

  if (this->ParentGroup)
  {
    int pos = this->ParentGroup->findItemPosition(name.toStdString());
    valid &= pos < 0;
  }

  return valid;
}

// ------------------------------------------------------------------------
void ItemDefDialog::setEditMode(EditMode mode)
{
  bool enable = true;
  using QDBB = QDialogButtonBox;
  QDBB::StandardButtons buttons = QDBB::Cancel | QDBB::Apply;
  switch (mode)
  {
    case EditMode::NEW:
      this->Ui->cbTypes->setEnabled(enable);
      this->Ui->leName->setEnabled(enable);
      connect(
        this->Ui->cbTypes, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(const int)));
      this->onTypeChanged(this->Ui->cbTypes->currentIndex());
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
      std::cerr << "Error: Invalid edit mode!\n";
  }

  this->Ui->leLabel->setEnabled(enable);
  this->Ui->leVersion->setEnabled(enable);
  this->Ui->gbConcrete->setEnabled(enable);
  this->buttonBox()->setStandardButtons(buttons);
}

// ------------------------------------------------------------------------
void ItemDefDialog::onTypeChanged(const int type)
{
  this->Handler = HandlerItemDef::create(type);

  delete this->Ui->gbConcrete;
  this->Ui->gbConcrete = new QGroupBox(this);
  this->Ui->wPlaceholder->layout()->addWidget(this->Ui->gbConcrete);

  using SMTKItem = smtk::attribute::Item;
  const QString title =
    QString::fromStdString(SMTKItem::type2String(static_cast<SMTKItem::Type>(type))) +
    " Properties";
  this->Ui->gbConcrete->setTitle(title);

  this->Handler->initialize(nullptr, this->Ui->gbConcrete);
}

// ------------------------------------------------------------------------
QStringList ItemDefDialog::getTypeList()
{
  QStringList names;
  using namespace smtk::attribute;
  for (int type = 0; type < Item::NUMBER_OF_TYPES; type++)
  {
    names << QString::fromStdString(Item::type2String(static_cast<Item::Type>(type)));
  }

  return names;
}
