//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtPreviewArc.h"

#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/resource/PersistentObject.h"

#include "smtk/operation/Launcher.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/StringItem.h"

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

qtPreviewArc::qtPreviewArc(
  qtDiagramScene* scene,
  const std::shared_ptr<smtk::operation::Manager>& operationManager,
  smtk::string::Token arcType,
  const std::string& arcOperation,
  const std::string& arcDestinationItemName,
  QGraphicsItem* parent)
  : Superclass(parent)
  , m_scene(scene)
  , m_arcType(arcType)
  , m_arcOperation(arcOperation)
  , m_arcDestinationItemName(arcDestinationItemName)
  , m_operationManager(operationManager)
{
  const auto& cfg(*m_scene->configuration());
  this->setAcceptedMouseButtons(Qt::NoButton);

  m_predecessor = nullptr;
  m_successor = nullptr;
  this->createOperation();

  m_scene->addItem(this);

  // === Task-specific constructor ===
  this->setZValue(cfg.constructionLayer()); // Draw construction geometry on top of nodes.
}

qtPreviewArc::~qtPreviewArc()
{
  m_scene->removeItem(this);
}

QRectF qtPreviewArc::boundingRect() const
{
  const auto& cfg(*m_scene->configuration());
  const qreal margin = cfg.arcWidth() + cfg.arcOutline();

  QRectF pb;
  if (m_predecessor && !m_successor)
  {
    pb = m_predecessorMark.boundingRect();
  }
  else
  {
    pb = this->path().boundingRect();
    pb = pb.united(m_predecessorMark.boundingRect());
    pb = pb.united(m_successorMark.boundingRect());
  }
  return pb.adjusted(-margin, -margin, margin, margin);
}

qtPreviewArc::ArcStatus qtPreviewArc::arcStatusEnum(const std::string& enumerant, bool* match)
{
  if (match)
  {
    *match = true;
  }
  std::string arcStatusName(enumerant);
  std::transform(
    arcStatusName.begin(), arcStatusName.end(), arcStatusName.begin(), [](unsigned char c) {
      return std::tolower(c);
    });
  if (arcStatusName.substr(0, 6) == "smtk::")
  {
    arcStatusName = arcStatusName.substr(6);
  }
  if (arcStatusName.substr(0, 11) == "extension::")
  {
    arcStatusName = arcStatusName.substr(11);
  }
  if (arcStatusName.substr(0, 11) == "qtpreviewarc::")
  {
    arcStatusName = arcStatusName.substr(11);
  }
  if (arcStatusName.substr(0, 9) == "arctype::")
  {
    arcStatusName = arcStatusName.substr(9);
  }
  if (arcStatusName == "invalid")
  {
    return ArcStatus::Invalid;
  }
  if (match)
  {
    *match = (arcStatusName == "valid");
  }
  return ArcStatus::Valid;
}

std::string qtPreviewArc::arcStatusName(ArcStatus enumerant)
{
  switch (enumerant)
  {
    case ArcStatus::Invalid:
      return "invalid";
    case ArcStatus::Valid:
      return "valid";
  }
  return "unknown";
}

void qtPreviewArc::setPredecessor(qtBaseObjectNode* predecessor)
{
  if (m_predecessor == predecessor)
  {
    return;
  }
  if (m_predecessor)
  {
    QObject::disconnect(
      m_predecessor, &qtBaseObjectNode::nodeMoved, this, &qtPreviewArc::updateArcPoints);
    QObject::disconnect(
      m_predecessor, &qtBaseObjectNode::nodeResized, this, &qtPreviewArc::updateArcPoints);
    if (m_operation)
    {
      m_operation->parameters()->disassociate(m_predecessor->object()->shared_from_this());
    }
  }
  m_predecessorConfirmed = false;
  m_canAssociatePredecessor = false;
  if (!predecessor)
  {
    // Do not allow successor without predecessor.
    this->setSuccessor(nullptr, false);
  }
  m_predecessor = predecessor;
  if (m_predecessor)
  {
    m_canAssociatePredecessor = m_operation
      ? m_operation->parameters()->associate(m_predecessor->object()->shared_from_this())
      : false;
    if (m_canAssociatePredecessor)
    {
      // TODO:
      // Association succeeded but we may have more stringent requirements than the operation.
    }
    QObject::connect(
      m_predecessor, &qtBaseObjectNode::nodeMoved, this, &qtPreviewArc::updateArcPoints);
    QObject::connect(
      m_predecessor, &qtBaseObjectNode::nodeResized, this, &qtPreviewArc::updateArcPoints);
  }
  if (m_canAssociatePredecessor && m_successor)
  {
    m_ableToOperate = m_operation->ableToOperate();
  }
  // Update the path
  this->updateArcPoints();
}

void qtPreviewArc::setSuccessor(qtBaseObjectNode* successor, bool updatePoints)
{
  if ((m_successor == successor) || (successor && !m_predecessor) || successor == m_predecessor)
  {
    // Value would be unchanged or you tried to set the successor without a valid predecessor.
    return;
  }
  if (m_successor)
  {
    QObject::disconnect(
      m_successor, &qtBaseObjectNode::nodeMoved, this, &qtPreviewArc::updateArcPoints);
    QObject::disconnect(
      m_successor, &qtBaseObjectNode::nodeResized, this, &qtPreviewArc::updateArcPoints);
    if (m_operation)
    {
      auto successorItem =
        m_operation ? m_operation->parameters()->findReference(m_arcDestinationItemName) : nullptr;
      if (successorItem)
      {
        successorItem->unset();
      }
    }
  }
  m_canAssociateSuccessor = false;
  m_successor = successor;
  if (m_successor)
  {
    auto successorItem =
      m_operation ? m_operation->parameters()->findReference(m_arcDestinationItemName) : nullptr;
    if (successorItem)
    {
      if (!successorItem->numberOfValues())
      {
        successorItem->setNumberOfValues(1);
      }
      m_canAssociateSuccessor = successorItem->setValue(m_successor->object()->shared_from_this());
      if (m_canAssociateSuccessor)
      {
        // TODO:
        // Insertion succeeded but we may have more stringent requirements than the operation.
      }
    }
    QObject::connect(
      m_successor, &qtBaseObjectNode::nodeMoved, this, &qtPreviewArc::updateArcPoints);
    QObject::connect(
      m_successor, &qtBaseObjectNode::nodeResized, this, &qtPreviewArc::updateArcPoints);
  }
  m_ableToOperate = m_canAssociateSuccessor ? m_operation->ableToOperate() : false;
  // Update the path
  if (updatePoints)
  {
    this->updateArcPoints();
  }
}

void qtPreviewArc::confirmPredecessorNode(bool confirm)
{
  if (!m_predecessor)
  {
    return;
  }
  m_predecessorConfirmed = confirm;
}

void qtPreviewArc::confirmSuccessorNode()
{
  if (!m_ableToOperate)
  {
    return;
  }
  // Launch the operation.
  m_operationManager->launchers()(m_operation);
  // Reset our predecessor to the successor and invalidate other ivars.
  m_predecessorConfirmed = false;
  m_successor = nullptr;
  // Start a new arc at the destination of the first:
  m_predecessor = m_successor;
  // Create a new operation so we don't overwrite the one we just launched.
  this->createOperation();
}

int qtPreviewArc::updateArcPoints()
{
  const auto& cfg(*m_scene->configuration());
  this->prepareGeometryChange();
  m_predecessorMark.clear();
  m_successorMark.clear();
  m_computedPath.clear();
  m_arrowPath.clear();

  if (!m_predecessor)
  {
    this->setPath(m_computedPath);
    return 1;
  }

  auto predRect = m_predecessor->boundingRect();
  predRect = m_predecessor->mapRectToScene(predRect);
  auto pc = predRect.center();
  m_predecessorMark.addRoundedRect(
    predRect, 2 * cfg.nodeBorderThickness(), 2 * cfg.nodeBorderThickness());
  // m_predecessorMark.addEllipse(pc, predRect.height()/4, predRect.height()/4);

  if (!m_successor)
  {
    // The only path is the predecessor mark. Clear out the arc path.
    this->setPath(m_computedPath);
    return 1;
  }

  auto succRect = m_successor->boundingRect();
  succRect = m_successor->mapRectToScene(succRect);
  auto sc = succRect.center();
  // m_successorMark.addEllipse(sc, succRect.height()/4, succRect.height()/4);
  m_successorMark.addRoundedRect(
    succRect, 2 * cfg.nodeBorderThickness(), 2 * cfg.nodeBorderThickness());

  // If the nodes overlap, there is no arc to draw.
  if (predRect.intersects(succRect))
  {
    this->setPath(m_computedPath);
    return 1;
  }

  // Determine a line connecting the node centers.
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

bool qtPreviewArc::setArcType(
  smtk::string::Token arcType,
  const std::string& operationType,
  const std::string& destinationItemName)
{
  if (
    arcType == m_arcType && m_arcOperation == operationType &&
    m_arcDestinationItemName == destinationItemName)
  {
    // Do nothing.
    return false;
  }

  if (!m_operationManager)
  {
    // We cannot create any operations.
    return false;
  }

  if (arcType.valid())
  {
    // Test that we can create the arc constructor operation.
    auto testOp = m_operationManager->create(operationType);
    if (!testOp)
    {
      return false;
    }
  }

  m_operation = nullptr;
  m_arcType = arcType;
  m_arcOperation = operationType;
  m_arcDestinationItemName = destinationItemName;

  this->createOperation();
  return true;
}

void qtPreviewArc::createOperation()
{
  m_canAssociatePredecessor = false;
  m_canAssociateSuccessor = false;
  m_ableToOperate = false;
  if (m_operationManager && !m_arcOperation.empty())
  {
    m_operation = m_operationManager->create(m_arcOperation);
    if (m_operation)
    {
      if (m_predecessor)
      {
        m_canAssociatePredecessor =
          m_operation->parameters()->associate(m_predecessor->object()->shared_from_this());
        if (m_canAssociatePredecessor)
        {
          // TODO:
          // Association succeeded but we may have more stringent requirements than the operation.
        }
      }
      if (m_successor)
      {
        auto successorItem = m_operation->parameters()->findReference(m_arcDestinationItemName);
        if (successorItem)
        {
          m_canAssociateSuccessor =
            successorItem->setValue(m_successor->object()->shared_from_this());
          if (m_canAssociateSuccessor)
          {
            // TODO:
            // Insertion succeeded but we may have more stringent requirements than the operation.
          }
        }
      }
      auto arcTypeItem = m_operation->parameters()->findString("arc type");
      if (arcTypeItem)
      {
        arcTypeItem->setValue(m_arcType.data());
      }
      m_ableToOperate = m_operation->ableToOperate();
    }
  }
  // Now the arc color might have changed, so update the path+pen.
  this->updateArcPoints();
}

void qtPreviewArc::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  (void)option;
  (void)widget;
  const auto& cfg(*m_scene->configuration());

  QPen fromPen;
  QPen toPen;
  QPen arcPen;
  fromPen.setWidth(cfg.arcWidth());
  fromPen.setBrush(
    cfg.colorForArcStatus(m_canAssociatePredecessor ? ArcStatus::Valid : ArcStatus::Invalid));
  toPen.setWidth(cfg.arcWidth());
  toPen.setBrush(
    cfg.colorForArcStatus(m_canAssociateSuccessor ? ArcStatus::Valid : ArcStatus::Invalid));
  arcPen.setWidth(cfg.arcWidth() * 2);
  arcPen.setBrush(cfg.colorForArcStatus(m_ableToOperate ? ArcStatus::Valid : ArcStatus::Invalid));

  // painter->setPen(pen);
  painter->strokePath(this->path(), arcPen);
  painter->fillPath(m_arrowPath, arcPen.brush());

  painter->strokePath(m_predecessorMark, fromPen);
  painter->fillPath(m_predecessorMark, fromPen.brush());
  painter->strokePath(m_successorMark, toPen);
  painter->fillPath(m_successorMark, toPen.brush());
}

} // namespace extension
} // namespace smtk
