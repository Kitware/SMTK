//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtModelView.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtAssociationWidget.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/model/Model.h"
#include "smtk/model/Item.h"
#include "smtk/model/GroupItem.h"
#include "smtk/view/Model.h"

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
#include <QPointer>

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtModelViewInternals
{
public:
  QPointer<QListWidget> ListBox;
  QPointer<QFrame> topFrame;
  QPointer<QFrame> bottomFrame;
  QPointer<qtAssociationWidget> AssociationsWidget;
  std::vector<smtk::attribute::DefinitionPtr> attDefs;
};

//----------------------------------------------------------------------------
qtModelView::
qtModelView(smtk::view::BasePtr dataObj, QWidget* p, qtUIManager* uiman) :
  qtBaseView(dataObj, p, uiman)
{
  this->Internals = new qtModelViewInternals;
  this->createWidget( );
}

//----------------------------------------------------------------------------
qtModelView::~qtModelView()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
const std::vector<smtk::attribute::DefinitionPtr> &qtModelView::attDefinitions() const
{
  return this->Internals->attDefs;
}

//----------------------------------------------------------------------------
void qtModelView::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  smtk::view::ModelPtr mview =
    smtk::dynamic_pointer_cast<smtk::view::Model>(this->getObject());
  if(!mview)
    {
    return;
    }

  Manager *attManager = this->uiManager()->attManager();
  smtk::model::MaskType mask = mview->modelEntityMask();
  if(mask != 0 && !mview->definition())
    {
    attManager->findDefinitions(mask, this->Internals->attDefs);
    }

  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  //this panel looks better in a over / under layout, rather than left / right
  frame->setOrientation( Qt::Vertical );

  QFrame* topFrame = new QFrame(frame);
  QFrame* bottomFrame = new QFrame(frame);

  this->Internals->topFrame = topFrame;
  this->Internals->bottomFrame = bottomFrame;

  QVBoxLayout* leftLayout = new QVBoxLayout(topFrame);
  leftLayout->setMargin(0);
  QVBoxLayout* rightLayout = new QVBoxLayout(bottomFrame);
  rightLayout->setMargin(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a list box for all the entries
  this->Internals->ListBox = new QListWidget(topFrame);
  this->Internals->ListBox->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Internals->AssociationsWidget = new qtAssociationWidget(bottomFrame, this);
  rightLayout->addWidget(this->Internals->AssociationsWidget);

  leftLayout->addWidget(this->Internals->ListBox);

  frame->addWidget(topFrame);
  frame->addWidget(bottomFrame);

  // if there is a definition, the view should
  // display all model entities of the requested mask along
  // with the attribute of this type in a table view
  attribute::DefinitionPtr attDef = mview->definition();
  if(attDef)
    {
    this->Internals->attDefs.push_back(attDef);
    }

  // signals/slots
  QObject::connect(this->Internals->ListBox,
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )),
    this, SLOT(onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * )));

  this->Widget = frame;
  if(this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(frame);
    }
  this->updateModelAssociation();
}

//----------------------------------------------------------------------------
void qtModelView::updateModelAssociation()
{
  bool isRegion = this->isRegionDomain();
  this->Internals->topFrame->setVisible(!isRegion);
  if(!isRegion)
    {
    this->updateModelEntityItems();
    }
  this->onShowCategory();
}
//----------------------------------------------------------------------------
bool qtModelView::isRegionDomain()
{
  smtk::view::ModelPtr mview =
    smtk::dynamic_pointer_cast<smtk::view::Model>(this->getObject());
  if(!mview)
    {
    return false;
    }
  return false;
}

//----------------------------------------------------------------------------
void qtModelView::updateModelEntityItems()
{
  this->Internals->ListBox->blockSignals(true);
  this->Internals->ListBox->clear();

  smtk::view::ModelPtr mview =
    smtk::dynamic_pointer_cast<smtk::view::Model>(this->getObject());
  if(!mview)
    {
    this->Internals->ListBox->blockSignals(false);
    return;
    }
  if(smtk::model::MaskType mask = mview->modelEntityMask())
    {
    smtk::model::ModelPtr refModel = this->uiManager()->attManager()->refModel();
    std::vector<smtk::model::GroupItemPtr> result=refModel->findGroupItems(mask);
    std::vector<smtk::model::GroupItemPtr>::iterator it = result.begin();
    for(; it!=result.end(); ++it)
      {
      this->addModelEntityItem(*it);
      }
    }
  if(this->Internals->ListBox->count())
    {
    this->Internals->ListBox->setCurrentRow(0);
    }
  this->Internals->ListBox->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtModelView::onShowCategory()
{
  if(this->isRegionDomain())
    {
    smtk::view::ModelPtr mview =
      smtk::dynamic_pointer_cast<smtk::view::Model>(this->getObject());
    smtk::model::MaskType mask = mview->modelEntityMask() ?
      mview->modelEntityMask() : static_cast<smtk::model::MaskType>(smtk::model::Item::REGION);
    smtk::model::ModelPtr refModel = this->uiManager()->attManager()->refModel();
    std::vector<smtk::model::GroupItemPtr> result(refModel->findGroupItems(mask));
    this->Internals->AssociationsWidget->showDomainsAssociation(
      result, this->Internals->attDefs);
    }
  else
    {
    smtk::model::ItemPtr theItem = this->getSelectedModelEntityItem();
    if(theItem)
      {
      this->Internals->AssociationsWidget->showAttributeAssociation(
        theItem, this->Internals->attDefs);
      }
    }
}
//----------------------------------------------------------------------------
void qtModelView::onListBoxSelectionChanged(
  QListWidgetItem * /*current*/, QListWidgetItem * /*previous*/)
{
  this->onShowCategory();
}
//-----------------------------------------------------------------------------
smtk::model::ItemPtr qtModelView::getSelectedModelEntityItem()
{
  return this->getModelEntityItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
smtk::model::ItemPtr qtModelView::getModelEntityItem(
  QListWidgetItem * item)
{
  smtk::model::Item* rawPtr = item ?
    static_cast<smtk::model::Item*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->shared_from_this() : smtk::model::ItemPtr();
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtModelView::getSelectedItem()
{
  return this->Internals->ListBox->currentItem();
}
//----------------------------------------------------------------------------
QListWidgetItem* qtModelView::addModelEntityItem(
  smtk::model::ItemPtr childData)
{
  QListWidgetItem* item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      this->Internals->ListBox, smtk_USER_DATA_TYPE);
  QVariant vdata;
  vdata.setValue(static_cast<void*>(childData.get()));
  item->setData(Qt::UserRole, vdata);
//  item->setFlags(item->flags() | Qt::ItemIsEditable);
  this->Internals->ListBox->addItem(item);
  return item;
}
