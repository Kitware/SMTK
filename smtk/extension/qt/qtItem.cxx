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
#include "smtk/extension/qt/qtItem.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtOverlay.h"

#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include <QPointer>
#include <QLayout>
#include <QComboBox>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtItemInternals
{
public:
  qtItemInternals(smtk::attribute::ItemPtr dataObject, QWidget* p,
   qtBaseView* bview)
  {
  this->ParentWidget = p;
  this->DataObject = dataObject;
  this->BaseView = bview;
  }
  ~qtItemInternals()
  {
  }
 smtk::attribute::WeakItemPtr DataObject;
 QPointer<QWidget> ParentWidget;
 QList<smtk::attribute::qtItem*> ChildItems;
 QPointer<qtBaseView> BaseView;
 QPointer<qtOverlayFilter> advOverlay;
 QPointer<QComboBox> AdvLevelCombo;
};


//----------------------------------------------------------------------------
qtItem::qtItem(smtk::attribute::ItemPtr dataObject, QWidget* p, qtBaseView* bview)
{
  this->Internals  = new qtItemInternals(dataObject, p, bview);
  this->Widget = NULL;
  this->IsLeafItem = false;

  //this->Internals->DataConnect = NULL;
  //this->createWidget();
}

//----------------------------------------------------------------------------
qtItem::~qtItem()
{
  this->clearChildItems();
  if (this->Internals)
    {
    if(this->Internals->ParentWidget && this->Widget
      && this->Internals->ParentWidget->layout())
      {
      this->Internals->ParentWidget->layout()->removeWidget(this->Widget);
      }
    delete this->Internals;
    }
}

//----------------------------------------------------------------------------
qtBaseView* qtItem::baseView()
{
  return this->Internals->BaseView;
}

//----------------------------------------------------------------------------
void qtItem::updateItemData()
{
//  if(this->widget() && this->baseView()->advanceLevelVisible())
//    {
//    this->showAdvanceLevelOverlay(true);
//    }
}

//----------------------------------------------------------------------------
void qtItem::addChildItem(qtItem* child)
{
  if(!this->Internals->ChildItems.contains(child))
    {
    this->Internals->ChildItems.append(child);
    }
}
//----------------------------------------------------------------------------
QList<qtItem*>& qtItem::childItems() const
{
  return this->Internals->ChildItems;
}
//----------------------------------------------------------------------------
void qtItem::clearChildItems()
{
  for(int i=0; i < this->Internals->ChildItems.count(); i++)
    {
    delete this->Internals->ChildItems.value(i);
    }
  this->Internals->ChildItems.clear();
}

//----------------------------------------------------------------------------
smtk::attribute::ItemPtr qtItem::getObject()
{
  return this->Internals->DataObject.lock();
}

//----------------------------------------------------------------------------
QWidget* qtItem::parentWidget()
{
  return this->Internals->ParentWidget;
}

//----------------------------------------------------------------------------
bool qtItem::passAdvancedCheck()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  return this->baseView()->uiManager()->passAdvancedCheck(
    dataObj->advanceLevel());
}

//----------------------------------------------------------------------------
void qtItem::showAdvanceLevelOverlay(bool show)
{
  if(!this->widget())
    {
    return;
    }
  if(show && !this->Internals->advOverlay)
    {
    this->Internals->advOverlay = new qtOverlayFilter(this->widget(), this);
    this->Internals->AdvLevelCombo = new QComboBox(
        this->Internals->advOverlay->overlay());
    this->Internals->advOverlay->overlay()->addOverlayWidget(
      this->Internals->AdvLevelCombo);
    this->baseView()->uiManager()->initAdvanceLevels(
      this->Internals->AdvLevelCombo);
    int mylevel = this->getObject()->advanceLevel(0);
    bool foundLevel = false;
    std::set<int> levels;
    for(int i=0; i<this->Internals->AdvLevelCombo->count(); i++)
      {
      int level = this->Internals->AdvLevelCombo->itemData(i).toInt();
      if(level == mylevel)
        {
        this->Internals->AdvLevelCombo->setCurrentIndex(i);
        foundLevel = true;
        break;
        }
      levels.insert(level);
      }
    // in case the advanceLevel is not one of the "AdvanceLevels" specified in xml,
    // we use the lowest level that is more than mylevel.
    // for example, levels 100, 200, 300.
    // mylevel 50 will use level 100
    // mylevel 150 will use level 200
    // my level 250 will use level 300
    // my level 350 will not be visible at all
    if(!foundLevel && levels.size() > 0)
      {
      levels.insert(mylevel);
      std::set<int>::iterator it = levels.upper_bound(mylevel);
      mylevel = it == levels.end() ? *(--it) : *it;
      int idx = std::distance(levels.begin(), it) - 1;
      this->Internals->AdvLevelCombo->setCurrentIndex(idx);
      }
    const double* rgba = this->baseView()->uiManager()->
      attManager()->advanceLevelColor(mylevel);
    if(rgba)
      {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
      }

    QObject::connect(this->Internals->AdvLevelCombo,
      SIGNAL(currentIndexChanged(int)), this, SLOT(onAdvanceLevelChanged(int)));
    }

  if(this->Internals->advOverlay)
    {
    this->Internals->advOverlay->setActive(show);
    }
  if(show)
    {
    this->widget()->repaint();
    }
}

//----------------------------------------------------------------------------
void qtItem::onAdvanceLevelChanged(int levelIdx)
{
  int level = this->Internals->AdvLevelCombo->itemData(levelIdx).toInt();
  this->setAdvanceLevel(level);
}

//----------------------------------------------------------------------------
void qtItem::setAdvanceLevel(int l)
{
  this->getObject()->setAdvanceLevel(0, l);
  this->getObject()->setAdvanceLevel(1, l);
  if(this->Internals->advOverlay)
    {
    const double* rgba = this->baseView()->uiManager()->
      attManager()->advanceLevelColor(l);
    if(rgba)
      {
      this->Internals->advOverlay->overlay()->setColor(
        QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]));
      this->Internals->advOverlay->overlay()->repaint();
      }
    }
}
