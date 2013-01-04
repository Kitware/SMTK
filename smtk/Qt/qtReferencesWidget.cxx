/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "smtk/Qt/qtReferencesWidget.h"

#include "smtk/Qt/qtUIManager.h"
#include "smtk/Qt/qtTableWidget.h"
#include "smtk/Qt/qtAttribute.h"
#include "smtk/Qt/qtItem.h"

#include "smtk/attribute/ModelEntitySection.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Manager.h"
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
void qtReferencesWidget::showAdvanced(int checked)
{
}

//----------------------------------------------------------------------------
void qtReferencesWidget::showAttributeReferences(
  smtk::AttributePtr att, QString& category)
{
  this->Internals->CurrentList->blockSignals(true);
  this->Internals->AvailableList->blockSignals(true);
  this->Internals->CurrentList->clear();
  this->Internals->AvailableList->clear();

  if(att)
    {
    // Lets see what attributes are being referenced
    std::vector<smtk::AttributeItemPtr> refs;
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
  QListWidgetItem * current, QListWidgetItem * previous)
{
}

//----------------------------------------------------------------------------
void qtReferencesWidget::onAvailableListSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
}

//-----------------------------------------------------------------------------
smtk::AttributePtr qtReferencesWidget::getSelectedAttribute(
  QListWidget* theList)
{
  return this->getAttributeFromItem(this->getSelectedItem(theList));
}
//-----------------------------------------------------------------------------
smtk::AttributePtr qtReferencesWidget::getAttributeFromItem(
  QListWidgetItem * item)
{
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : smtk::AttributePtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtReferencesWidget::getSelectedItem(QListWidget* theList)
{
  return theList->selectedItems().count()>0 ?
    theList->selectedItems().value(0) : NULL;
}
//----------------------------------------------------------------------------
QListWidgetItem* qtReferencesWidget::addAttributeRefListItem(
  QListWidget* theList, smtk::AttributeItemPtr refItem)
{
  QString txtLabel(refItem->attribute()->name().c_str());
  txtLabel.append(" : ").append(refItem->owningItem()->name().c_str());

  QListWidgetItem* item = new QListWidgetItem(txtLabel,
      theList, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue((void*)(refItem.get()));
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
