//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtTaskView.h"

#include "smtk/extension/qt/task/qtBaseTaskNode.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskScene.h"

#include "smtk/io/Logger.h"

#include <QAction>
#include <QByteArray>
#include <QDragEnterEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QWheelEvent>

namespace smtk
{
namespace extension
{

using namespace smtk::string::literals;

class qtTaskView::Internal
{
public:
  Internal() = default;
  Internal(qtTaskView* self, qtTaskEditor* editor)
    : m_self(self)
    , m_editor(editor)
  {
  }
  Internal(const Internal&) = delete;
  void operator=(const Internal&) = delete;

  qtTaskView* m_self{ nullptr };
  qtTaskEditor* m_editor{ nullptr };
  smtk::string::Token m_snapBackMode;
  Qt::Key m_snapBackKey;
};

qtTaskView::qtTaskView(qtTaskScene* scene, qtTaskEditor* widget)
  : Superclass(scene, widget->widget())
  , m_p(new Internal(this, widget))
{
  this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  this->setDragMode(QGraphicsView::ScrollHandDrag);
  this->setRubberBandSelectionMode(Qt::IntersectsItemShape);
  this->setAcceptDrops(true);
  constexpr QRectF MAX_SCENE_SIZE{ -1e4, -1e4, 3e4, 3e4 };
  this->setSceneRect(MAX_SCENE_SIZE);
}

qtTaskView::~qtTaskView()
{
  delete m_p;
  m_p = nullptr;
}

qtTaskEditor* qtTaskView::taskEditor() const
{
  return m_p ? m_p->m_editor : nullptr;
}

void qtTaskView::addModeSnapback(Qt::Key snapBackOnReleaseKey, smtk::string::Token snapToMode)
{
  if (!m_p || !m_p->m_editor || !snapToMode.valid())
  {
    return;
  }

  auto curMode = m_p->m_editor->mode();
  if (curMode == snapToMode)
  {
    return;
  }

  m_p->m_snapBackMode = curMode;
  m_p->m_snapBackKey = snapBackOnReleaseKey;
  m_p->m_editor->requestModeChange(snapToMode);
}

void qtTaskView::wheelEvent(QWheelEvent* event)
{
  constexpr double ZOOM_INCREMENT_RATIO = 0.0125;

  const ViewportAnchor anchor = this->transformationAnchor();
  this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  const int angle = event->angleDelta().y();
  static ulong lastTimestamp = 0;
  double dt =
    (event->timestamp() > lastTimestamp && lastTimestamp != 0 ? event->timestamp() - lastTimestamp
                                                              : 50);
  lastTimestamp = event->timestamp();
  double rate = std::abs(angle / dt);
  const double factor = 1.0 + rate * ((angle > 0) ? ZOOM_INCREMENT_RATIO : -ZOOM_INCREMENT_RATIO);

  this->scale(factor, factor);
  this->setTransformationAnchor(anchor);
}

void qtTaskView::keyPressEvent(QKeyEvent* event)
{
  this->Superclass::keyPressEvent(event);
}

void qtTaskView::keyReleaseEvent(QKeyEvent* event)
{
  if (m_p->m_snapBackMode.valid() && event->key() == m_p->m_snapBackKey)
  {
    m_p->m_editor->requestModeChange(m_p->m_snapBackMode);
    m_p->m_snapBackMode = smtk::string::Token();
    m_p->m_snapBackKey = Qt::Key_unknown;
  }
  else
  {
    this->Superclass::keyReleaseEvent(event);
  }
}

void qtTaskView::mouseMoveEvent(QMouseEvent* event)
{
  this->Superclass::mouseMoveEvent(event);
}

void qtTaskView::mousePressEvent(QMouseEvent* event)
{
  this->Superclass::mousePressEvent(event);
}

void qtTaskView::mouseReleaseEvent(QMouseEvent* event)
{
  this->Superclass::mouseReleaseEvent(event);
}

void qtTaskView::mouseDoubleClickEvent(QMouseEvent* event)
{
  this->Superclass::mouseDoubleClickEvent(event);
}

void qtTaskView::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasFormat("application/x-smtk-worklet-name"))
  {
    event->acceptProposedAction();
  }
  // TODO: Add a fancy preview of laid-out nodes+arcs to be inserted upon drop.
}

void qtTaskView::dragLeaveEvent(QDragLeaveEvent* event)
{
  (void)event;
  // TODO: Remove a fancy preview of laid-out nodes+arcs to be inserted upon drop.
}

void qtTaskView::dragMoveEvent(QDragMoveEvent* event)
{
  (void)event;
  // TODO: Translate the fancy preview of laid-out nodes+arcs to be inserted upon drop.
}

void qtTaskView::dropEvent(QDropEvent* event)
{
  bool didAdd = false;
  bool didFail = false;
  std::array<double, 2> location{ { event->posF().x(), event->posF().y() } };
  QByteArray encodedData = event->mimeData()->data("application/x-smtk-worklet-name");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  while (!stream.atEnd())
  {
    QString text;
    stream >> text;
    if (m_p->m_editor && m_p->m_editor->addWorklet(text.toStdString(), location))
    {
      didAdd = true;
    }
    else
    {
      didFail = true;
    }
  }
  if (didAdd)
  {
    event->acceptProposedAction();
  }
  if (didFail)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Failed to emplace one or more dropped worklets.");
  }
}

} // namespace extension
} // namespace smtk
