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
#include "AttributeBrowser.h"
#include "InputDialog.h"
#include "ui_AttributeBrowser.h"
#include "ui_DefinitionsForm.h"

//------------------------------------------------------------------------------
AttributeBrowser::AttributeBrowser(QWidget* parent)
  : QDockWidget(parent)
  , Ui(new Ui::AttributeBrowser)
{
  this->Ui->setupUi(this);

  QObject::connect(this->Ui->pbAddDefinition, SIGNAL(clicked()), this, SLOT(onAddDefinition()));
  QObject::connect(this->Ui->pbDelDefinition, SIGNAL(clicked()), this, SLOT(onDeleteDefinition()));
}

//------------------------------------------------------------------------------
AttributeBrowser::~AttributeBrowser() = default;

//------------------------------------------------------------------------------
void AttributeBrowser::populate(smtk::attribute::SystemPtr system)
{
  this->populateDefinitions(system);
  //this->populateAnaylsis();
  //this->populateCategories();
  //this->populateViews();
}

//------------------------------------------------------------------------------
void AttributeBrowser::clear()
{
}

//------------------------------------------------------------------------------
void AttributeBrowser::populateDefinitions(smtk::attribute::SystemPtr system)
{
  if (this->AttDefModel)
  {
    delete this->AttDefModel;
  }
  this->AttDefModel = new AttDefDataModel(this);

  this->AttDefModel->populate(system);
  this->Ui->viewDefinitions->setModel(this->AttDefModel);
  this->Ui->viewDefinitions->expandAll();

  QItemSelectionModel* sm = this->Ui->viewDefinitions->selectionModel();

  connect(sm, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this,
    SLOT(onAttDefSelectionChanged(const QModelIndex&, const QModelIndex&)));

  const QModelIndex defaultIndex = this->AttDefModel->getDefaultIndex();
  sm->setCurrentIndex(defaultIndex, QItemSelectionModel::Select);
}

//------------------------------------------------------------------------------
void AttributeBrowser::onAttDefSelectionChanged(
  const QModelIndex& current, const QModelIndex& previous)
{
  // Disable deletion if other definitions derive from current
  this->Ui->pbDelDefinition->setEnabled(!this->AttDefModel->hasDerivedTypes(current));
  emit attDefChanged(current, previous);
}

//------------------------------------------------------------------------------
void AttributeBrowser::onAddDefinition()
{
  InputDialog dialog(this);

  using FormPtr = std::unique_ptr<Ui::DefinitionsForm>;
  FormPtr defUi = FormPtr(new Ui::DefinitionsForm);
  defUi->setupUi(dialog.centralWidget());
  defUi->tvTypes->setHeaderHidden(true);
  defUi->tvTypes->setModel(AttDefModel);
  defUi->tvTypes->expandAll();
  // TODO Set selection to the currently selected in UI->viewDefinitions

  if (dialog.exec() == QDialog::Accepted)
  {
    // TODO handle cbRootType
    AttDefDataModel::DefProperties props;
    props.Type = defUi->leType->text().toStdString();
    QItemSelectionModel* sm = defUi->tvTypes->selectionModel();
    props.BaseType = this->AttDefModel->getAttDefType(sm->currentIndex());

    props.IsUnique = defUi->cbUnique->isChecked();
    props.IsAbstract = defUi->cbAbstract->isChecked();
    props.Label = defUi->leLabel->text().toStdString();

    this->AttDefModel->addAttDef(props);
  }
}

//------------------------------------------------------------------------------
void AttributeBrowser::onDeleteDefinition()
{
  auto sm = this->Ui->viewDefinitions->selectionModel();
  auto attDefIndex = sm->currentIndex();

  this->AttDefModel->removeAttDef(attDefIndex);
}
