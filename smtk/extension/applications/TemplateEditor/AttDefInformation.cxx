//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QModelIndex>

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ItemDefinition.h"

#include "AttDefDataModel.h"
#include "AttDefInformation.h"
#include "InputDialog.h"
#include "ItemDefinitionHelper.h"
#include "ItemDefinitionsDataModel.h"
#include "ui_AttDefInformation.h"
#include "ui_ItemDefInputForm.h"
#include "ui_ItemDefinitionForm.h" //TODO rename ItemDefInfoForm

// -----------------------------------------------------------------------------
AttDefInformation::AttDefInformation(QWidget* parent)
  : QWidget(parent)
  , Ui(new Ui::AttDefInformation)
  , InheritedItemDefModel(new ItemDefinitionsDataModel(this))
  , OwnedItemDefModel(new ItemDefinitionsDataModel(this))
{
  this->Ui->setupUi(this);

  connect(this->Ui->pbSaveDef, SIGNAL(clicked()), this, SLOT(onSaveAttDef()));
  connect(this->Ui->pbAddItemDef, SIGNAL(clicked()), this, SLOT(onAddItemDef()));
  connect(this->Ui->pbDeleteItemDef, SIGNAL(clicked()), this, SLOT(onRemoveItemDef()));

  this->Ui->tvInheritedItems->setExpandsOnDoubleClick(false);
  this->Ui->tvOwnedItems->setExpandsOnDoubleClick(false);
  connect(this->Ui->tvInheritedItems, SIGNAL(doubleClicked(const QModelIndex&)), this,
    SLOT(showInheritedItemDetails(const QModelIndex&)));
  connect(this->Ui->tvOwnedItems, SIGNAL(doubleClicked(const QModelIndex&)), this,
    SLOT(showOwnedItemDetails(const QModelIndex&)));
}

// -----------------------------------------------------------------------------
AttDefInformation::~AttDefInformation() = default;

// -----------------------------------------------------------------------------
void AttDefInformation::onAttDefChanged(
  const QModelIndex& currentDef, const QModelIndex& previousDef)
{
  this->updateAttDefData(currentDef);
  this->updateInheritedItemDef();
  this->updateOwnedItemDef();
}

// -----------------------------------------------------------------------------
void AttDefInformation::updateAttDefData(const QModelIndex& currentDef)
{
  AttDefDataModel const* model = qobject_cast<AttDefDataModel const*>(currentDef.model());
  this->CurrentAttDef = model->getAttDef(currentDef);

  auto baseDef = this->CurrentAttDef->baseDefinition();
  QString baseDefStr = baseDef ? QString::fromStdString(baseDef->type()) : "";

  this->Ui->laType->setText(QString::fromStdString(this->CurrentAttDef->type()));
  this->Ui->laBaseType->setText(baseDefStr);
  this->Ui->leLabel->setText(QString::fromStdString(this->CurrentAttDef->label()));
  this->Ui->cbUnique->setChecked(this->CurrentAttDef->isUnique());
  this->Ui->cbAbstract->setChecked(this->CurrentAttDef->isAbstract());
}

// -----------------------------------------------------------------------------
void AttDefInformation::updateOwnedItemDef()
{
  // TODO Deleting is currently necessary only because removeRow()/clear() are
  // not implemented in the model. The model instance could be reused once these
  // are implemented.
  delete this->OwnedItemDefModel;
  this->OwnedItemDefModel = new ItemDefinitionsDataModel(this);
  this->OwnedItemDefModel->clear();
  this->OwnedItemDefModel->appendBranchToRoot(this->CurrentAttDef);
  this->Ui->tvOwnedItems->setModel(this->OwnedItemDefModel);
  this->Ui->tvOwnedItems->setColumnWidth(0, 250);
  this->Ui->tvOwnedItems->setColumnHidden(2, true);
  this->Ui->tvOwnedItems->expandAll();
}

// -----------------------------------------------------------------------------
void AttDefInformation::updateInheritedItemDef()
{
  delete this->InheritedItemDefModel;
  this->InheritedItemDefModel = new ItemDefinitionsDataModel(this);
  //this->InheritedItemDefModel->clear(); // TODO Instead of deleting the mod

  // Add inherited ItemDefinitions from each parent type recursively
  std::function<void(smtk::attribute::DefinitionPtr&)> recursiveAdd;
  recursiveAdd = [&recursiveAdd, this](smtk::attribute::DefinitionPtr& def) {
    smtk::attribute::DefinitionPtr baseDef = def->baseDefinition();
    if (baseDef)
    {
      this->InheritedItemDefModel->appendBranchToRoot(baseDef);
      recursiveAdd(baseDef);
    }
  };

  recursiveAdd(this->CurrentAttDef);

  this->Ui->tvInheritedItems->setModel(this->InheritedItemDefModel);
  this->Ui->tvInheritedItems->setColumnWidth(0, 250);
  this->Ui->tvInheritedItems->expandAll();
}

// -----------------------------------------------------------------------------
void AttDefInformation::onSaveAttDef()
{
  this->CurrentAttDef->setLabel(this->Ui->leLabel->text().toStdString());
  this->CurrentAttDef->setIsUnique(this->Ui->cbUnique->isChecked());
  this->CurrentAttDef->setIsAbstract(this->Ui->cbAbstract->isChecked());

  // TODO Update Associations
}

// -----------------------------------------------------------------------------
void AttDefInformation::onAddItemDef()
{
  InputDialog dialog(this);
  using FormPtr = std::unique_ptr<Ui::ItemDefInputForm>;
  FormPtr defUi = FormPtr(new Ui::ItemDefInputForm);
  defUi->setupUi(dialog.centralWidget());
  defUi->cbTypes->addItems(ItemDefinitionHelper::getTypes());

  if (dialog.exec() == QDialog::Accepted)
  {
    QItemSelectionModel* sm = this->Ui->tvOwnedItems->selectionModel();
    auto currentIndex = sm->currentIndex();
    auto itemDef = this->OwnedItemDefModel->getItemDef(currentIndex);

    ItemDefinitionsDataModel::ItemDefProperties props;
    props.Definition = this->CurrentAttDef;
    props.Type = defUi->cbTypes->currentText().toStdString();
    props.Name = defUi->leName->text().toStdString();
    props.ParentNode = currentIndex.parent();

    this->OwnedItemDefModel->insertItem(props);
  }
}

// -----------------------------------------------------------------------------
void AttDefInformation::onRemoveItemDef()
{
  // TODO
}

// -----------------------------------------------------------------------------
void AttDefInformation::showInheritedItemDetails(const QModelIndex& index)
{
  InputDialog dialog(this);

  using FormPtr = std::unique_ptr<Ui::ItemDefinitionForm>;
  FormPtr defUi = FormPtr(new Ui::ItemDefinitionForm);
  defUi->setupUi(dialog.centralWidget());

  // Fill up form with current values
  ItemDefinitionsDataModel const* model =
    qobject_cast<ItemDefinitionsDataModel const*>(index.model());
  auto const& itemDef = model->getItemDef(index);

  defUi->laType->setText(
    QString::fromStdString(smtk::attribute::Item::type2String(itemDef->type())));
  defUi->leName->setText(QString::fromStdString(itemDef->name()));
  defUi->leLabel->setText(QString::fromStdString(itemDef->label()));
  defUi->leVersion->setText(QString::number(itemDef->version()));
  defUi->leAdvanceLevel->setText(QString::number(itemDef->advanceLevel()));

  if (dialog.exec() == QDialog::Accepted)
  {
    // TODO save values
  }
}

// -----------------------------------------------------------------------------
void AttDefInformation::showOwnedItemDetails(const QModelIndex& index)
{
  /// TODO Temporary hack to show the dialog.
  this->showInheritedItemDetails(index);
}
