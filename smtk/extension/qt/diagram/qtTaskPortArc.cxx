//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// This must come first in order for <cmath> to define M_PI on win32.
#define _USE_MATH_DEFINES

#include "smtk/extension/qt/diagram/qtTaskPortArc.h"

#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskPortNode.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/task/Adaptor.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QLayout>
#include <QPainter>

#include <cmath>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtTaskPortArc::qtTaskPortArc(
  qtDiagramGenerator* generator,
  qtBaseObjectNode* predecessor,
  qtTaskPortNode* successor,
  QGraphicsItem* parent)
  : Superclass(generator, predecessor, successor, "port connection", parent)
{
  this->updateArcPoints();

  this->scene()->addItem(this);

  // === Port-specific constructor ===
  const auto& cfg(*this->scene()->configuration());
  this->setZValue(cfg.arcLayer() + 1); // Draw dependencies on top of adaptors
}

qtTaskPortArc::~qtTaskPortArc() = default;

smtk::resource::PersistentObject* qtTaskPortArc::predecessorObject() const
{
  auto* node = dynamic_cast<qtBaseObjectNode*>(this->predecessor());
  if (!node)
  {
    return nullptr;
  }
  return node->object();
}

smtk::task::Port* qtTaskPortArc::successorPort() const
{
  auto* node = dynamic_cast<qtTaskPortNode*>(this->successor());
  if (!node)
  {
    return nullptr;
  }
  return node->port();
}

int qtTaskPortArc::updateArcPoints()
{
  const auto& cfg(*this->scene()->configuration());
  (void)cfg;
  this->prepareGeometryChange();
  m_computedPath.clear();
  m_arrowPath.clear();

  auto predRect = m_predecessor->boundingRect();
  predRect = m_predecessor->mapRectToScene(predRect);
  auto succRect = m_successor->boundingRect();
  succRect = m_successor->mapRectToScene(succRect);

  qreal pang = -m_predecessor->rotation() * M_PI / 180.;
  // Because the successor must always be an input port,
  // we flip the incoming angle by 180 degrees:
  qreal sang = (-m_successor->rotation() + 180.0) * M_PI / 180.;

  // If the nodes overlap, there is no arc to draw.
  if (predRect.intersects(succRect))
  {
    this->setPath(m_computedPath);
    return 1;
  }

  // Determine a line connecting the node centers.
  auto pc = predRect.center();
  auto sc = succRect.center();
  qreal predDiam = predRect.width() > predRect.height() ? predRect.width() : predRect.height();
  qreal succDiam = succRect.width() > succRect.height() ? succRect.width() : succRect.height();
  QPointF po(0, 0);
  po.setX(pc.x() + 1.2 * predDiam * std::cos(pang));
  po.setY(pc.y() + 1.2 * predDiam * -std::sin(pang));

  QPointF so(0, 0);
  so.setX(sc.x() + 1.2 * succDiam * std::cos(sang));
  so.setY(sc.y() + 1.2 * succDiam * -std::sin(sang));

  // Now declare our path as a rational cubic curve (pc po so sc):
  m_computedPath.moveTo(pc);
  m_computedPath.cubicTo(po, so, sc);
  this->setPath(m_computedPath);

  return 1;
}

} // namespace extension
} // namespace smtk
