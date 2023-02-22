//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtTaskArc.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/task/qtTaskNode.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QLayout>
#include <QPainter>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtTaskArc::qtTaskArc(
  qtTaskScene* scene,
  qtTaskNode* predecessor,
  qtTaskNode* successor,
  ArcType arcType,
  QGraphicsItem* parent)
  : Superclass(parent)
  , m_scene(scene)
  , m_predecessor(predecessor)
  , m_successor(successor)
  , m_arcType(arcType)
{
  const auto& cfg(*m_scene->configuration());
  QObject::connect(
    m_predecessor, &qtTaskNode::nodeMovedImmediate, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_predecessor, &qtTaskNode::nodeResized, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_successor, &qtTaskNode::nodeMovedImmediate, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_successor, &qtTaskNode::nodeResized, this, &qtTaskArc::updateArcPoints);
  this->setAcceptedMouseButtons(Qt::NoButton);

  this->updateArcPoints();

  m_scene->addItem(this);

  // === Task-specific constructor ===
  this->setZValue(cfg.arcLayer() + 1); // Draw dependencies on top of adaptors
}

qtTaskArc::qtTaskArc(
  qtTaskScene* scene,
  qtTaskNode* predecessor,
  qtTaskNode* successor,
  smtk::task::Adaptor* adaptor,
  QGraphicsItem* parent)
  : Superclass(parent)
  , m_scene(scene)
  , m_predecessor(predecessor)
  , m_successor(successor)
  , m_adaptor(adaptor)
  , m_arcType(ArcType::Adaptor)
{
  const auto& cfg(*m_scene->configuration());
  QObject::connect(m_predecessor, &qtTaskNode::nodeMoved, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_predecessor, &qtTaskNode::nodeResized, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_successor, &qtTaskNode::nodeMoved, this, &qtTaskArc::updateArcPoints);
  QObject::connect(m_successor, &qtTaskNode::nodeResized, this, &qtTaskArc::updateArcPoints);
  this->setAcceptedMouseButtons(Qt::NoButton);

  this->updateArcPoints();

  m_scene->addItem(this);

  // === Task-specific constructor ===
  this->setZValue(cfg.arcLayer());
}

qtTaskArc::~qtTaskArc()
{
  m_scene->removeItem(this);
}

QRectF qtTaskArc::boundingRect() const
{
  const auto& cfg(*m_scene->configuration());
  const qreal margin = cfg.arcWidth() + cfg.arcOutline();

  QRectF pb = this->path().boundingRect();
  return pb.adjusted(-margin, -margin, margin, margin);
}

qtTaskArc::ArcType qtTaskArc::arcTypeEnum(const std::string& enumerant, bool* match)
{
  if (match)
  {
    *match = true;
  }
  std::string arcTypeName(enumerant);
  std::transform(arcTypeName.begin(), arcTypeName.end(), arcTypeName.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  if (arcTypeName.substr(0, 6) == "smtk::")
  {
    arcTypeName = arcTypeName.substr(6);
  }
  if (arcTypeName.substr(0, 11) == "extension::")
  {
    arcTypeName = arcTypeName.substr(11);
  }
  if (arcTypeName.substr(0, 11) == "qttaskarc::")
  {
    arcTypeName = arcTypeName.substr(11);
  }
  if (arcTypeName.substr(0, 9) == "arctype::")
  {
    arcTypeName = arcTypeName.substr(9);
  }
  if (arcTypeName == "dependency")
  {
    return ArcType::Dependency;
  }
  if (arcTypeName == "adaptor")
  {
    return ArcType::Adaptor;
  }
  if (match)
  {
    *match = (arcTypeName == "dependency");
  }
  return ArcType::Dependency;
}

std::string qtTaskArc::arcTypeName(ArcType enumerant)
{
  switch (enumerant)
  {
    case ArcType::Dependency:
      return "dependency";
    case ArcType::Adaptor:
      return "adaptor";
  }
  return "unknown";
}

int qtTaskArc::updateArcPoints()
{
  const auto& cfg(*m_scene->configuration());
  this->prepareGeometryChange();
  m_computedPath.clear();
  m_arrowPath.clear();

  auto predRect = m_predecessor->boundingRect();
  predRect = m_predecessor->mapRectToScene(predRect);
  auto succRect = m_successor->boundingRect();
  succRect = m_successor->mapRectToScene(succRect);

  // If the nodes overlap, there is no arc to draw.
  if (predRect.intersects(succRect))
  {
    this->setPath(m_computedPath);
    return 1;
  }

  // Determine a line connecting the node centers.
  auto pc = predRect.center();
  auto sc = succRect.center();
  auto dl = sc - pc;                     // delta. Line is L(t) = pc + dl * t, t ∈ [0,1].
  auto dp = predRect.bottomRight() - pc; // Always positive along both axes.

  // Find point, pi, on boundary of predRect and on line bet. pc and sc
  qreal tx = dp.x() / std::abs(dl.x());
  qreal ty = dp.y() / std::abs(dl.y());
  QPointF pi;
  pi = pc + (tx < ty ? tx : ty) * dl;
  // ni is the "normal" to the node at the intersection point pi.
  QPointF ni(
    tx < ty ? (std::signbit(dl.x()) ? -1. : +1.) : 0.,
    tx < ty ? 0. : (std::signbit(dl.y()) ? -1. : +1));
  QPointF nearestCorner(
    dl.x() < 0. ? (dl.y() < 0. ? predRect.topLeft() : predRect.bottomLeft())
                : (dl.y() < 0. ? predRect.topRight() : predRect.bottomRight()));
  QPointF cornerVector(nearestCorner - pi);
  // Make the normal transition smoothly near corners:
  if (
    (tx >= ty && std::abs(cornerVector.x()) < cfg.arrowStemLength()) ||
    (tx < ty && std::abs(cornerVector.y()) < cfg.arrowStemLength()))
  {
    ni = pi -
      (nearestCorner + QPointF(std::signbit(dl.x()) ? +16 : -16, std::signbit(dl.y()) ? +16 : -16));
    qreal invMag = 1.0 / std::sqrt(QPointF::dotProduct(ni, ni));
    ni = invMag * ni;

    // Adjust the intersection point to deal with the rounded rectangle corner:
    if (
      (tx >= ty && std::abs(cornerVector.x()) < cfg.nodeRadius()) ||
      (tx < ty && std::abs(cornerVector.y()) < cfg.nodeRadius()))
    {
      QPointF centerOfCurvature =
        nearestCorner + QPointF(std::signbit(dl.x()) ? +4 : -4, std::signbit(dl.y()) ? +4 : -4);
      QPointF tmp = pi - centerOfCurvature;
      qreal tmpMag = std::sqrt(QPointF::dotProduct(tmp, tmp));
      pi = centerOfCurvature + (4.0 / tmpMag) * tmp;
    }
  }

  // Now, telescope a couple points out along the normal.
  QPointF pi1 = pi + ni * cfg.arrowStemLength();
  QPointF pi2 = pi + ni * cfg.arrowStemLength() * 2;

  // Repeat the above, but with the successor node (so dl is reversed).
  dl = -1.0 * dl;
  auto ds = succRect.bottomRight() - sc; // Always positive along both axes.

  tx = ds.x() / std::abs(dl.x());
  ty = ds.y() / std::abs(dl.y());
  QPointF si;
  si = sc + (tx < ty ? tx : ty) * dl;
  // ni is the "normal" to the node at the intersection point si.
  ni = QPointF(
    tx < ty ? (std::signbit(dl.x()) ? -1. : +1.) : 0.,
    tx < ty ? 0. : (std::signbit(dl.y()) ? -1. : +1));
  nearestCorner = QPointF(
    dl.x() < 0. ? (dl.y() < 0. ? succRect.topLeft() : succRect.bottomLeft())
                : (dl.y() < 0. ? succRect.topRight() : succRect.bottomRight()));
  cornerVector = QPointF(nearestCorner - si);
  // Make the normal transition smoothly near corners:
  if (
    (tx >= ty && std::abs(cornerVector.x()) < cfg.arrowStemLength()) ||
    (tx < ty && std::abs(cornerVector.y()) < cfg.arrowStemLength()))
  {
    ni = si -
      (nearestCorner + QPointF(std::signbit(dl.x()) ? +16 : -16, std::signbit(dl.y()) ? +16 : -16));
    qreal invMag = 1.0 / std::sqrt(QPointF::dotProduct(ni, ni));
    ni = invMag * ni;

    // Adjust the intersection point to deal with the rounded rectangle corner:
    if (
      (tx >= ty && std::abs(cornerVector.x()) < cfg.nodeRadius()) ||
      (tx < ty && std::abs(cornerVector.y()) < cfg.nodeRadius()))
    {
      QPointF centerOfCurvature =
        nearestCorner + QPointF(std::signbit(dl.x()) ? +4 : -4, std::signbit(dl.y()) ? +4 : -4);
      QPointF tmp = si - centerOfCurvature;
      qreal tmpMag = std::sqrt(QPointF::dotProduct(tmp, tmp));
      si = centerOfCurvature + (4.0 / tmpMag) * tmp;
    }
  }

  // Now, telescope a couple points out along the normal.
  QPointF si1 = si + ni * cfg.arrowStemLength();
  QPointF si2 = si + ni * cfg.arrowStemLength() * 2;

  // Midpoint between pi and si
  QPointF psm = 0.5 * (pi1 + si1);

  // Arrowhead points that replace si as the path's destination
  // 12 = 0.75 * ARC_ARROW_STEM = ARC_ARROW_HEAD
  //  6 = 0.50 * ARC_ARROW_HEAD
  QPointF a1 = si + 12. * ni;
  QPointF ti{ ni.y(), -ni.x() };
  QPointF a2 = a1 + 6. * ti;
  QPointF a3 = a1 - 6. * ti;
  QPointF a4 = 0.75 * a1 + 0.25 * si;
  // Finally, we can declare our path:
  // (pi pi1) [pi2 psm si2] (si1 a4)
  // The points in parentheses are connected with a straight line.
  // The points in square brackets use a rational quadratic curve.
  m_computedPath.moveTo(pi);
  m_computedPath.lineTo(pi1);
  m_computedPath.quadTo(pi2, psm);
  m_computedPath.quadTo(si2, si1);
  m_computedPath.lineTo(a4);
  this->setPath(m_computedPath);

  m_arrowPath.moveTo(si);
  m_arrowPath.lineTo(a2);
  m_arrowPath.quadTo(a4, a3);
  m_arrowPath.lineTo(si);
  return 1;
}

void qtTaskArc::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  (void)option;
  (void)widget;
  const auto& cfg(*m_scene->configuration());

  QPen pen;
  pen.setWidth(cfg.arcWidth());
  pen.setBrush(cfg.colorForArc(m_arcType));

  // painter->setPen(pen);
  painter->strokePath(this->path(), pen);
  painter->fillPath(m_arrowPath, pen.brush());
}

} // namespace extension
} // namespace smtk
