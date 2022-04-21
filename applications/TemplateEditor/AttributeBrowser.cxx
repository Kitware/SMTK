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

#include "AttDefDataModel.h"
#include "AttDefDialog.h"
#include "AttributeBrowser.h"
#include "ui_AttributeBrowser.h"

//------------------------------------------------------------------------------
AttributeBrowser::AttributeBrowser(QWidget* parent)
  : QDockWidget(parent)
  , Ui(new Ui::AttributeBrowser)
{
  this->Ui->setupUi(this);

  connect(this->Ui->pbAddDefinition, SIGNAL(clicked()), this, SLOT(onAddDefinition()));
  connect(
    this->Ui->viewDefinitions,
    SIGNAL(showDialog(const QModelIndex&)),
    this,
    SLOT(onAddDefinition()));

  connect(this->Ui->pbDelDefinition, SIGNAL(clicked()), this, SLOT(onDeleteDefinition()));

  connect(
    this->Ui->leSearch,
    SIGNAL(textChanged(const QString&)),
    this,
    SLOT(onSearchAttDef(const QString&)));
}

//------------------------------------------------------------------------------
AttributeBrowser::~AttributeBrowser() = default;

//------------------------------------------------------------------------------
void AttributeBrowser::populate(smtk::attribute::ResourcePtr resource)
{
  this->populateDefinitions(resource);
  //this->populateAnaylsis();
  //this->populateCategories();
  //this->populateViews();
}

//------------------------------------------------------------------------------
void AttributeBrowser::clear() {}

//------------------------------------------------------------------------------
void AttributeBrowser::populateDefinitions(smtk::attribute::ResourcePtr resource)
{
  delete this->AttDefModel;
  this->AttDefModel = new AttDefDataModel(this);

  this->AttDefModel->populate(resource);
  this->Ui->viewDefinitions->setModel(this->AttDefModel);
  this->Ui->viewDefinitions->expandAll();

  QItemSelectionModel* sm = this->Ui->viewDefinitions->selectionModel();

  connect(
    sm,
    SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
    this,
    SLOT(onAttDefSelectionChanged(const QModelIndex&, const QModelIndex&)));

  const QModelIndex defaultIndex = this->AttDefModel->getDefaultIndex();
  sm->setCurrentIndex(defaultIndex, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------------
void AttributeBrowser::onAttDefSelectionChanged(
  const QModelIndex& current,
  const QModelIndex& previous)
{
  // Disable deletion if other definitions derive from current
  this->Ui->pbDelDefinition->setEnabled(!this->AttDefModel->hasDerivedTypes(current));
  Q_EMIT attDefChanged(current, previous);
}

//------------------------------------------------------------------------------
void AttributeBrowser::emitAttDefChanged()
{
  auto* sm = this->Ui->viewDefinitions->selectionModel();
  const auto index = sm->currentIndex();
  Q_EMIT attDefChanged(index, index);
}

//------------------------------------------------------------------------------
void AttributeBrowser::onAddDefinition()
{
  AttDefDialog dialog(this);
  const auto defIndex = this->Ui->viewDefinitions->selectionModel()->currentIndex();
  dialog.setBaseAttDef(this->AttDefModel->get(defIndex));

  if (dialog.exec() == QDialog::Accepted)
  {
    this->AttDefModel->insert(dialog.getInputValues());
    Q_EMIT resourceChanged(true);
  }
}

//------------------------------------------------------------------------------
void AttributeBrowser::onDeleteDefinition()
{
  auto* sm = this->Ui->viewDefinitions->selectionModel();
  auto attDefIndex = sm->currentIndex();

  this->AttDefModel->remove(attDefIndex);
  Q_EMIT resourceChanged(true);
}

//------------------------------------------------------------------------------
void AttributeBrowser::onSearchAttDef(const QString& text)
{
  if (text.isEmpty())
  {
    // Set model's default index
    QItemSelectionModel* sm = this->Ui->viewDefinitions->selectionModel();
    const QModelIndex defaultIndex = this->AttDefModel->getDefaultIndex();
    sm->setCurrentIndex(defaultIndex, QItemSelectionModel::Select);
  }

  this->Ui->viewDefinitions->keyboardSearch(text);
}
