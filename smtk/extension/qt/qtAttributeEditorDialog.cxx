//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeEditorDialog - A Information Dialog for SMTK Operations
// .SECTION Description
// .SECTION Caveats

#include "smtk/extension/qt/qtAttributeEditorDialog.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "ui_qtAttributeEditorWidget.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/view/Configuration.h"

#include <QMessageBox>
#include <QPushButton>

using namespace smtk::extension;

qtAttributeEditorDialog::qtAttributeEditorDialog(
  const smtk::attribute::AttributePtr& attribute,
  smtk::extension::qtUIManager* uiManager,
  QWidget* Parent)
  : QDialog(Parent)
  , m_attribute(attribute)
  , m_uiManager(uiManager)
  , m_widget(new Ui::qtAttributeEditorWidget())
{
  m_widget->setupUi(this);
  this->setObjectName("qtAttributeEditorDialog");

  m_instancedViewDef = smtk::view::Configuration::New("Instanced", "Contents");
  smtk::view::Configuration::Component& child =
    m_instancedViewDef->details().addChild("InstancedAttributes").addChild("Att");
  child.setAttribute("Name", m_attribute->name()).setAttribute("Type", m_attribute->type());

  smtk::view::Information v;
  v.insert<smtk::view::ConfigurationPtr>(m_instancedViewDef);
  v.insert<QWidget*>(this->m_widget->attributeFrame);
  v.insert<smtk::extension::qtUIManager*>(m_uiManager);
  v.insert<std::weak_ptr<smtk::attribute::Resource>>(m_attribute->attributeResource());
  qtInstancedView* iview = dynamic_cast<qtInstancedView*>(qtInstancedView::createViewWidget(v));
  m_instancedView.reset(iview);

  m_widget->attributeName->setText(m_attribute->name().c_str());
  m_widget->editAccept->setDefault(false);
  m_widget->editAccept->setAutoDefault(false);
  m_widget->editCancel->setDefault(false);
  m_widget->editCancel->setAutoDefault(false);
  QObject::connect(
    m_widget->attributeName, SIGNAL(editingFinished()), this, SLOT(attributeNameChanged()));

  QObject::connect(m_widget->editAccept, SIGNAL(pressed()), this, SLOT(accept()));

  QObject::connect(m_widget->editCancel, SIGNAL(pressed()), this, SLOT(reject()));
}

qtAttributeEditorDialog::~qtAttributeEditorDialog()
{
  delete m_widget;
}

void qtAttributeEditorDialog::hideCancel()
{
  m_widget->editCancel->hide();
}

void qtAttributeEditorDialog::showCancel()
{
  m_widget->editCancel->show();
}

void qtAttributeEditorDialog::attributeNameChanged()
{
  std::string newName = m_widget->attributeName->text().toStdString();
  if (newName == m_attribute->name())
  {
    return;
  }

  auto attResource = m_attribute->attributeResource();
  // Lets see if the name is in use
  auto att = attResource->findAttribute(newName);
  if (att != nullptr)
  {
    std::string s;
    s = "Can't rename " + m_attribute->type() + ":" + m_attribute->name() +
      ".  There already exists an attribute of type: " + att->type() + " named " + att->name() +
      ".";

    QMessageBox::warning(this, "Attribute Can't be Renamed", s.c_str());
    m_widget->attributeName->setText(m_attribute->name().c_str());
    return;
  }
  attResource->rename(m_attribute, newName);
}
