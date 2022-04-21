//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// self includes
#include "smtk/extension/qt/qtColorButton.h"

// Qt includes
#include <QColorDialog>
#include <QPainter>
#include <QResizeEvent>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

using namespace smtk::attribute;

qtColorButton::qtColorButton(QWidget* p)
  : QToolButton(p)
{
  //this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  //this->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->connect(this, SIGNAL(clicked()), SLOT(chooseColor()));
}

QColor qtColorButton::chosenColor() const
{
  return this->Color;
}

void qtColorButton::setChosenColor(const QColor& color)
{
  if (color.isValid())
  {
    if (color != this->Color)
    {
      this->Color = color;
      this->setIcon(this->renderColorSwatch(color));

      Q_EMIT this->chosenColorChanged(this->Color);
    }
    Q_EMIT this->validColorChosen(this->Color);
  }
}

QIcon qtColorButton::renderColorSwatch(const QColor& color)
{
  int radius = qRound(this->height() * 0.75);
  if (radius <= 10)
  {
    radius = 10;
  }

  QPixmap pix(radius, radius);
  pix.fill(QColor(0, 0, 0, 0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setBrush(QBrush(color));
  painter.drawEllipse(1, 1, radius - 2, radius - 2);
  painter.end();

  return QIcon(pix);
}

void qtColorButton::chooseColor()
{
  this->setChosenColor(
    QColorDialog::getColor(this->Color, this, "Choose Color", QColorDialog::DontUseNativeDialog));
}

void qtColorButton::resizeEvent(QResizeEvent* rEvent)
{
  (void)rEvent;

  this->setIcon(this->renderColorSwatch(this->Color));
}
