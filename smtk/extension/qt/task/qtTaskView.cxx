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

#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskScene.h"

#include <QAction>
#include <QKeyEvent>
#include <QWheelEvent>

namespace smtk
{
namespace extension
{

class qtTaskView::Internal
{
};

qtTaskView::qtTaskView(qtTaskScene* scene, qtTaskEditor* widget)
  : Superclass(scene, widget->widget())
  , m_p(new Internal)
{
  this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  this->setDragMode(QGraphicsView::ScrollHandDrag);
  constexpr QRectF MAX_SCENE_SIZE{ -1e4, -1e4, 3e4, 3e4 };
  this->setSceneRect(MAX_SCENE_SIZE);
}

qtTaskView::~qtTaskView()
{
  delete m_p;
  m_p = nullptr;
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

void qtTaskView::keyReleaseEvent(QKeyEvent* event)
{
  (void)event;
  // do nothing yet.
}

} // namespace extension
} // namespace smtk
