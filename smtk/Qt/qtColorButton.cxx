/*=========================================================================

   Program: ParaView
   Module:    qtColorButton.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

// self includes
#include "smtk/Qt/qtColorButton.h"

// Qt includes
#include <QColorDialog>
#include <QPainter>
#include <QResizeEvent>

#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

using namespace smtk::attribute;

//-----------------------------------------------------------------------------
qtColorButton::qtColorButton(QWidget* p)
  : QToolButton(p)
{
  this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  this->connect(this, SIGNAL(clicked()), SLOT(chooseColor()));
}

//-----------------------------------------------------------------------------
QColor qtColorButton::chosenColor() const
{
  return this->Color;
}

//-----------------------------------------------------------------------------
void qtColorButton::setChosenColor(const QColor& color)
{
  if(color.isValid())
    {
    if(color != this->Color)
      {
      this->Color = color;
      this->setIcon(this->renderColorSwatch(color));
      
      emit this->chosenColorChanged(this->Color);
      }
    emit this->validColorChosen(this->Color);
    }
}

//-----------------------------------------------------------------------------
QIcon qtColorButton::renderColorSwatch(const QColor& color)
{
  int radius = qRound(this->height() * 0.75);
  if (radius <= 10)
    {
    radius = 10;
    }

  QPixmap pix(radius, radius);
  pix.fill(QColor(0,0,0,0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setBrush(QBrush(color));
  painter.drawEllipse(1, 1, radius-2, radius-2);
  painter.end();

  return QIcon(pix);
}

//-----------------------------------------------------------------------------
void qtColorButton::chooseColor()
{
  this->setChosenColor(QColorDialog::getColor(this->Color, this));
}

//-----------------------------------------------------------------------------
void qtColorButton::resizeEvent(QResizeEvent *rEvent)
{
  (void) rEvent;

  this->setIcon(this->renderColorSwatch(this->Color));
}
