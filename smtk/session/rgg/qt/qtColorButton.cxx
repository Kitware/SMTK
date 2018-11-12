//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/qt/qtColorButton.h"

#include <QColor>
#include <QColorDialog>
#include <QPainter>
#include <QPushButton>

#include <cmath>

static double goldenRatio = 0.618033988749895f;
int qtColorButton::s_colorIndex = 1;

qtColorButton::qtColorButton(QWidget* parent, QColor defaultColor)
  : QPushButton(parent)
{
  QObject::connect(this, &qtColorButton::clicked, this, &qtColorButton::chooseColor);
  this->setColor(defaultColor);
}

qtColorButton::~qtColorButton()
{
}

QColor qtColorButton::getColor()
{
  return this->m_color;
}

void qtColorButton::getColor(std::vector<double>& color)
{
  color.clear();
  color.push_back(this->m_color.redF());
  color.push_back(this->m_color.greenF());
  color.push_back(this->m_color.blueF());
  color.push_back(this->m_color.alphaF());
}

void qtColorButton::chooseColor()
{
  QColor selected =
    QColorDialog::getColor(this->m_color, this, "Select color", QColorDialog::DontUseNativeDialog);
  if (selected.isValid())
  {
    this->setColor(selected);
  }
}

void qtColorButton::setColor(QColor color)
{
  this->setText(QString());
  this->m_color = color;
  this->setIcon(this->renderColorSwatch(color));
  emit this->colorModified();
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

QColor qtColorButton::generateColor()
{
  // TODO: Qt5.10 introduces qRandomGenerator class which is superuseful to generate
  // random color.
  double currentHue = s_colorIndex * goldenRatio;
  currentHue = std::fmod(currentHue, 1.0);
  s_colorIndex++;
  return QColor::fromHslF(currentHue, 1.0, 0.5);
}
