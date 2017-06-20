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

#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "HandlerVoid.h"
#include "ui_ItemDefRefForm.h"

// -----------------------------------------------------------------------------
bool HandlerVoid::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerVoid::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerVoid::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::VoidItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
HandlerRef::HandlerRef()
  : Ui(new Ui::ItemDefRefForm)
{
}

HandlerRef::~HandlerRef() = default;

bool HandlerRef::initialize_impl(QWidget* parent)
{
  this->Ui->setupUi(parent);
  QObject::connect(this->Ui->cbCommonLabel, SIGNAL(toggled(bool)), this->Ui->leCommonLabel,
    SLOT(setEnabled(bool)));

  if (this->ItemDef)
  {
    const auto item = std::static_pointer_cast<smtk::attribute::RefItemDefinition>(this->ItemDef);

    this->Ui->leNumReqValues->setText(QString::number(item->numberOfRequiredValues()));

    const bool useCommonLabel = item->usingCommonLabel();
    this->Ui->cbCommonLabel->setChecked(useCommonLabel);
    this->Ui->leCommonLabel->setVisible(useCommonLabel);
    this->Ui->leCommonLabel->setText(QString::fromStdString(item->valueLabel(0)));
  }
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerRef::updateItemDef_impl()
{
  auto item = std::static_pointer_cast<smtk::attribute::RefItemDefinition>(this->ItemDef);

  item->setNumberOfRequiredValues(static_cast<size_t>(this->Ui->leNumReqValues->text().toInt()));

  if (this->Ui->cbCommonLabel->isChecked())
  {
    item->setCommonValueLabel(this->Ui->leCommonLabel->text().toStdString());
  }

  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerRef::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::RefItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerFile::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerFile::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerFile::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::FileItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerDirectory::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDirectory::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDirectory::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DirectoryItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerModelEntity::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerModelEntity::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerModelEntity::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::ModelEntityItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerMeshSelection::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerMeshSelection::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerMeshSelection::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::MeshSelectionItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerMeshEntity::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerMeshEntity::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerMeshEntity::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::MeshItemDefinition::New(name);
}

////////////////////////////////////////////////////////////////////////////////
bool HandlerDateTime::initialize_impl(QWidget* parent)
{
  parent->hide();
  return true;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDateTime::updateItemDef_impl()
{
  return this->ItemDef;
}

// -----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr HandlerDateTime::createItemDef_impl(const std::string& name)
{
  return smtk::attribute::DateTimeItemDefinition::New(name);
}
