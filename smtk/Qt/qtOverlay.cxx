#include "qtOverlay.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QComboBox>
#include <QStringList>

using namespace smtk::attribute;

qtOverlay::qtOverlay(QWidget * parent ) : QWidget(parent)
{
  setAttribute(Qt::WA_NoSystemBackground);
  //setAttribute(Qt::WA_TransparentForMouseEvents);
  new QHBoxLayout(this);
  this->layout()->setAlignment(Qt::AlignRight);
  this->layout()->setMargin(0);
}

void qtOverlay::addOverlayWidget(QWidget*w)
{
  if(w)
    {
    w->setAttribute(Qt::WA_NoSystemBackground);
    w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    w->setStyleSheet(
      "QWidget { background-color: rgba(80, 80, 255, 128) }");
    this->layout()->addWidget(w);
    }
}

void qtOverlay::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.fillRect(rect(), QColor(80, 80, 255, 128));
}

//----------------------------------------------------------------------------
qtOverlayFilter::qtOverlayFilter(QWidget* onWidget, QObject * parent) : QObject(parent)
{
  m_Active = true;
  m_overlay = new qtOverlay(onWidget->parentWidget());
  m_overlay->setGeometry(onWidget->geometry());
  m_overlayOn = onWidget;
}
void qtOverlayFilter::setActive(bool val)
{
  this->m_overlay->setVisible(val);
  this->m_Active = val;
//  if(this->m_Active)
//    {
//    this->m_overlay->repaint();
//    }
}

void qtOverlayFilter::addOverlayWidget(QWidget*w)
{
    m_overlay->addOverlayWidget(w);
}

bool qtOverlayFilter::eventFilter(QObject * obj, QEvent * ev)
{
  if (!obj->isWidgetType())
    {
    return false;
    }
  if(this->m_overlay)
    {
    this->m_overlay->setVisible(this->m_Active);
    }
  if(!this->m_Active)
    {
    return false;
    }
  QWidget * w = static_cast<QWidget*>(obj);
  if (ev->type() == QEvent::Paint || ev->type() == QEvent::Show)
    {
    if (!m_overlay)
      {
      m_overlay = new qtOverlay(w->parentWidget());
      m_overlay->setGeometry(w->geometry());
      m_overlayOn = w;
      }
    m_overlay->show();
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
