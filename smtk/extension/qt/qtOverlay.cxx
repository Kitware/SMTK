//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOverlay.h"
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStringList>

using namespace smtk::extension;

qtOverlay::qtOverlay(QWidget* parentW)
  : QWidget(parentW)
{
  setAttribute(Qt::WA_NoSystemBackground);
  //setAttribute(Qt::WA_TransparentForMouseEvents);
  new QHBoxLayout(this);
  this->layout()->setAlignment(Qt::AlignRight);
  this->layout()->setMargin(0);
  m_overlayColor = QColor(80, 80, 255, 128);
}

void qtOverlay::addOverlayWidget(QWidget* w)
{
  if (w)
  {
    w->setAttribute(Qt::WA_NoSystemBackground);
    w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QString strStyle(" QWidget { background-color: rgba(");
    strStyle.append(QString::number(m_overlayColor.red()) + ", ")
      .append(QString::number(m_overlayColor.green()) + ", ")
      .append(QString::number(m_overlayColor.blue()) + ", ")
      .append(QString::number(m_overlayColor.alpha()) + ") } ");
    w->setStyleSheet(w->styleSheet() + strStyle);
    this->layout()->addWidget(w);
  }
}

void qtOverlay::paintEvent(QPaintEvent* /*event*/)
{
  QPainter p(this);
  p.fillRect(rect(), m_overlayColor);
}

qtOverlayFilter::qtOverlayFilter(QWidget* onWidget, QObject* parentO)
  : QObject(parentO)
{
  m_Active = true;
  m_overlay = new qtOverlay(onWidget->parentWidget());
  m_overlay->setGeometry(onWidget->geometry());
  m_overlayOn = onWidget;
  onWidget->installEventFilter(this);
}

qtOverlayFilter::~qtOverlayFilter()
{
  delete m_overlay;
}

void qtOverlayFilter::setActive(bool val)
{
  if (m_overlay && m_overlayOn)
  {
    m_overlay->setGeometry(m_overlayOn->geometry());
  }

  m_overlay->setVisible(val);
  m_Active = val;
}

void qtOverlayFilter::addOverlayWidget(QWidget* w)
{
  m_overlay->addOverlayWidget(w);
}

bool qtOverlayFilter::eventFilter(QObject* obj, QEvent* ev)
{
  if (!obj->isWidgetType())
  {
    return false;
  }
  if (m_overlay)
  {
    m_overlay->setVisible(m_Active);
  }
  if (!m_Active)
  {
    return false;
  }
  QWidget* w = static_cast<QWidget*>(obj);
  if (ev->type() == QEvent::Paint || ev->type() == QEvent::Show)
  {
    if (!m_overlay)
    {
      m_overlay = new qtOverlay(w->parentWidget());
      m_overlay->setGeometry(w->geometry());
      m_overlayOn = w;
    }
    if (m_overlay && m_overlayOn == w)
    {
      m_overlay->setGeometry(w->geometry());
      m_overlay->show();
      //      m_overlay->repaint();
    }
  }
  else if (ev->type() == QEvent::Resize)
  {
    if (m_overlay && m_overlayOn == w)
    {
      m_overlay->setGeometry(w->geometry());
    }
  }
  return false;
}
