//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtReferencesWidget.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <QGridLayout>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
#include <QSplitter>

#include "ui_qtAttributeAssociation.h"

namespace Ui { class qtAttributeAssociation; }

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtReferencesWidgetInternals : public Ui::qtAttributeAssociation
{
public:

};

//----------------------------------------------------------------------------
qtReferencesWidget::qtReferencesWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internals = new qtReferencesWidgetInternals;
  this->Internals->setupUi(this);

  this->initWidget( );
}

//----------------------------------------------------------------------------
qtReferencesWidget::~qtReferencesWidget()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtReferencesWidget::initWidget( )
{
  // signals/slots
  QObject::connect(this->Internals->CurrentList,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * )));
  QObject::connect(this->Internals->AvailableList,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * )));

  QObject::connect(this->Internals->MoveToRight,
    SIGNAL(clicked()), this, SLOT(onRemoveAssigned()));
  QObject::connect(this->Internals->MoveToLeft,
    SIGNAL(clicked()), this, SLOT(onAddAvailable()));
  QObject::connect(this->Internals->ExchangeLeftRight,
    SIGNAL(clicked()), this, SLOT(onExchange()));

}

//----------------------------------------------------------------------------
void qtReferencesWidget::showAttributeReferences(
  smtk::attribute::AttributePtr att, QString& /*category*/)
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if(att)
    {
    // Lets see what attributes are being referenced
    std::vector<smtk::attribute::ItemPtr> refs;
    std::size_t i;
    att->references(refs);
    for (i = 0; i < refs.size(); i++)
      {
      this->addAttributeRefListItem(
        this->Internals->CurrentList,refs[i]);
      }
    }
  this->Internals->CurrentList->blockSignals(false);
  this->Internals->AvailableList->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtReferencesWidget::onCurrentListSelectionChanged(
  QListWidgetItem * /*current*/, QListWidgetItem * /*previous*/)
{
}

//----------------------------------------------------------------------------
void qtReferencesWidget::onAvailableListSelectionChanged(
  QListWidgetItem * /*current*/, QListWidgetItem * /*previous*/)
{
}

//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtReferencesWidget::getSelectedAttribute(
  QListWidget* theList)
{
  return this->getAttributeFromItem(this->getSelectedItem(theList));
}
//-----------------------------------------------------------------------------
smtk::attribute::AttributePtr qtReferencesWidget::getAttributeFromItem(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ?
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::attribute::AttributePtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtReferencesWidget::getSelectedItem(QListWidget* theList)
{
  return theList->currentItem();
}
//----------------------------------------------------------------------------
QListWidgetItem* qtReferencesWidget::addAttributeRefListItem(
  QListWidget* theList, smtk::attribute::ItemPtr refItem)
{
  QString txtLabel(refItem->attribute()->name().c_str());
  txtLabel.append(" : ").append(refItem->owningItem()->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel,
      theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue(static_cast<void*>(refItem.get()));
  item->setData(Qt::UserRole, vdata);
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  theList->addItem(item);
  return item;
}
//----------------------------------------------------------------------------
void qtReferencesWidget::onRemoveAssigned()
{

}
//----------------------------------------------------------------------------
void qtReferencesWidget::onAddAvailable()
{

}
//----------------------------------------------------------------------------
void qtReferencesWidget::onExchange()
{

}
