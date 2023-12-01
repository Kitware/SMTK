//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramView.h"

#include "smtk/extension/qt/diagram/qtBaseTaskNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"

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

class qtDiagramView::Internal
{
public:
  Internal() = default;
  Internal(qtDiagramView* self, qtDiagram* diagram)
    : m_self(self)
    , m_diagram(diagram)
  {
  }
  Internal(const Internal&) = delete;
  void operator=(const Internal&) = delete;

  qtDiagramView* m_self{ nullptr };
  qtDiagram* m_diagram{ nullptr };
  smtk::string::Token m_snapBackMode;
  Qt::Key m_snapBackKey;

  // If a drop event is proposed, only 0 or 1 qtDiagramGenerators may accept it.
  // If one accepts, m_dropGenerator will refer to it until a dragLeaveEvent or
  // a dropEvent occurs.
  qtDiagramGenerator* m_dropGenerator{ nullptr };
};

qtDiagramView::qtDiagramView(qtDiagramScene* scene, qtDiagram* widget)
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

qtDiagramView::~qtDiagramView()
{
  delete m_p;
  m_p = nullptr;
}

qtDiagram* qtDiagramView::diagram() const
{
  return m_p ? m_p->m_diagram : nullptr;
}

void qtDiagramView::addModeSnapback(Qt::Key snapBackOnReleaseKey, smtk::string::Token snapToMode)
{
  if (!m_p || !m_p->m_diagram || !snapToMode.valid())
  {
    return;
  }

  auto curMode = m_p->m_diagram->mode();
  if (curMode == snapToMode)
  {
    return;
  }

  m_p->m_snapBackMode = curMode;
  m_p->m_snapBackKey = snapBackOnReleaseKey;
  m_p->m_diagram->requestModeChange(snapToMode);
}

void qtDiagramView::wheelEvent(QWheelEvent* event)
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

void qtDiagramView::keyPressEvent(QKeyEvent* event)
{
  this->Superclass::keyPressEvent(event);
}

void qtDiagramView::keyReleaseEvent(QKeyEvent* event)
{
  if (m_p->m_snapBackMode.valid() && event->key() == m_p->m_snapBackKey)
  {
    m_p->m_diagram->requestModeChange(m_p->m_snapBackMode);
    m_p->m_snapBackMode = smtk::string::Token();
    m_p->m_snapBackKey = Qt::Key_unknown;
  }
  else
  {
    this->Superclass::keyReleaseEvent(event);
  }
}

void qtDiagramView::mouseMoveEvent(QMouseEvent* event)
{
  this->Superclass::mouseMoveEvent(event);
}

void qtDiagramView::mousePressEvent(QMouseEvent* event)
{
  this->Superclass::mousePressEvent(event);
}

void qtDiagramView::mouseReleaseEvent(QMouseEvent* event)
{
  this->Superclass::mouseReleaseEvent(event);
}

void qtDiagramView::mouseDoubleClickEvent(QMouseEvent* event)
{
  this->Superclass::mouseDoubleClickEvent(event);
}

void qtDiagramView::dragEnterEvent(QDragEnterEvent* event)
{
  for (const auto& entry : m_p->m_diagram->generators())
  {
    if (entry.second->acceptDropProposal(event))
    {
      m_p->m_dropGenerator = entry.second.get();
      return;
    }
  }
}

void qtDiagramView::dragLeaveEvent(QDragLeaveEvent* event)
{
  if (m_p->m_dropGenerator)
  {
    m_p->m_dropGenerator->abortDrop(event);
  }
  m_p->m_dropGenerator = nullptr;
}

void qtDiagramView::dragMoveEvent(QDragMoveEvent* event)
{
  if (m_p->m_dropGenerator)
  {
    m_p->m_dropGenerator->moveDropPoint(event);
  }
}

void qtDiagramView::dropEvent(QDropEvent* event)
{
  if (m_p->m_dropGenerator)
  {
    m_p->m_dropGenerator->acceptDrop(event);
  }
  m_p->m_dropGenerator = nullptr;
}

} // namespace extension
} // namespace smtk
