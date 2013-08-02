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

#include "smtk/Qt/qtColorItem.h"
#include "smtk/Qt/qtColorButton.h"

#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QColor>

#include "smtk/attribute/ColorItem.h"
#include "smtk/attribute/ColorItemDefinition.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtColorItemInternals
{
public:

};


//----------------------------------------------------------------------------
qtColorItem::qtColorItem(
  smtk::AttributeItemPtr dataObj, QWidget* p) : qtItem(dataObj, p)
{
  this->Internals = new qtColorItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtColorItem::~qtColorItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtColorItem::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildItems();

  smtk::ColorItemPtr item =dynamicCastPointer<ColorItem>(this->getObject());
  if(!item)
    {
    return;
    }
  qtColorButton* colorBT = new qtColorButton(this->parentWidget());
  colorBT->setText(item->definition()->label().c_str());
  
  this->Widget = colorBT;
  this->updateItemData();

  QObject::connect(colorBT,  SIGNAL(chosenColorChanged(const QColor&)),
      this, SLOT(onColorChanged()), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
void qtColorItem::updateItemData()
{
  smtk::ColorItemPtr item =dynamicCastPointer<ColorItem>(this->getObject());
  if(!item)
    {
    return;
    }
  qtColorButton* colorBT = qobject_cast<qtColorButton*>(this->Widget);
  double r, g, b;
  item->getRGB(r, g, b);
  colorBT->setChosenColor(QColor::fromRgbF(r, g, b));
}
//----------------------------------------------------------------------------
void qtColorItem::onColorChanged()
{
  qtColorButton* const colorBT = qobject_cast<qtColorButton*>(
    QObject::sender());
  if(!colorBT)
    {
    return;
    }

  smtk::ColorItemPtr item =dynamicCastPointer<ColorItem>(this->getObject());
  QColor color = colorBT->chosenColor();
  item->setRGB(color.redF(), color.greenF(), color.blueF());
}
