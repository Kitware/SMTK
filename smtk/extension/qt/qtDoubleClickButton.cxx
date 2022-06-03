//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDoubleClickButton.h"

#include <QEvent>

qtDoubleClickButton::qtDoubleClickButton(const QIcon& icon, const QString& text, QWidget* parent)
  : Superclass(icon, text, parent)
{
  this->configure();
}

qtDoubleClickButton::qtDoubleClickButton(const QString& text, QWidget* parent)
  : Superclass(text, parent)
{
  this->configure();
}

qtDoubleClickButton::qtDoubleClickButton(QWidget* parent)
  : Superclass(parent)
{
  this->configure();
}

qtDoubleClickButton::~qtDoubleClickButton()
{
  m_timer.stop();
}

void qtDoubleClickButton::startTimer()
{
  if (m_timer.isActive())
  {
    m_timer.stop();
    this->doubleClicked();
  }
  else
  {
    m_timer.start(qtDoubleClickButton::timeout);
  }
}

void qtDoubleClickButton::configure()
{
  m_timer.setSingleShot(true);
  // When the QPushButton clicked() signal is emitted, start a timer
  // that may eventually fire *our* clicked() signal.
  QObject::connect(this, &Superclass::clicked, this, &qtDoubleClickButton::startTimer);
  // If the timer fires before more clicks, it was a single-click.
  QObject::connect(&m_timer, &QTimer::timeout, this, &qtDoubleClickButton::singleClicked);
}
