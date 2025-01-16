//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtResourceDiagramArc.h"

#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/qtBSpline2D.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QPainter>

#include <cmath>

// Set this to a non-zero value to add debug printouts.
#define SMTK_DBG_ARC 0

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtResourceDiagramArc::qtResourceDiagramArc(
  qtDiagramGenerator* generator,
  qtBaseNode* predecessor,
  qtBaseNode* successor,
  ArcType arcType,
  QGraphicsItem* parent)
  : Superclass(generator, predecessor, successor, arcType, parent)
{
  const auto& cfg(*this->scene()->configuration());
  this->updateArcPoints();

  this->scene()->addItem(this);

  // === Task-specific constructor ===
  this->setZValue(cfg.arcLayer() + 1); // Draw dependencies on top of adaptors
}

qtResourceDiagramArc::~qtResourceDiagramArc() = default;

int qtResourceDiagramArc::updateArcPoints()
{
  this->prepareGeometryChange();
  m_computedPath.clear();
  m_arrowPath.clear();

  if (!m_predecessor || !m_successor)
  {
    return 1;
  }

  auto* gen = reinterpret_cast<qtResourceDiagram*>(this->generator());
  // Now, find the shortest path up the predecessor's parents and down the successors parents.
  // These points will form the control points of our spline.
  std::vector<QPointF> controlPoints;
  {
    std::map<qtBaseNode*, int> upfwd;
    std::map<int, qtBaseNode*> upbck;
    std::vector<qtBaseNode*> downwards;
    qtBaseNode* nodeCursor = m_predecessor;
    int ii = 0;
    qtBaseNode* lca = nullptr;

    while (nodeCursor)
    {
      upfwd[nodeCursor] = ii;
      upbck[ii] = nodeCursor;
      ++ii;
      nodeCursor = dynamic_cast<qtBaseNode*>(nodeCursor->parentItem());
    }
    nodeCursor = m_successor;
    while (nodeCursor && upfwd.find(nodeCursor) == upfwd.end())
    {
      downwards.push_back(nodeCursor);
      nodeCursor = dynamic_cast<qtBaseNode*>(nodeCursor->parentItem());
    }
    if (nodeCursor)
    {
      lca = nodeCursor;                // Track the least-common ancestor.
      downwards.push_back(nodeCursor); // Push the least-common ancestor on the stack
    }
    auto it = nodeCursor ? upbck.find(upfwd[nodeCursor] - 1) : upbck.find(ii - 1);
    while (it != upbck.end() && it->first >= 0)
    {
      downwards.push_back(it->second);
      if (it->first == 0)
      {
        break;
      }
      --it;
    }

    // Scale the opacity of the arc so that short arcs are opaque and
    // long arcs have decreasing opacity.
    double exponent = std::fmin(0, 3.0 - static_cast<float>(downwards.size()));
    if (gen)
    {
      double sao = gen->shortArcOpacity();
      double lao = gen->longArcOpacityAdjustment();
      m_opacity = sao - lao * (1.0 - std::exp(exponent));
      // Clamp to [0,1]
      if (m_opacity > 1.0)
      {
        m_opacity = 1.0;
      }
      else if (m_opacity < 0.0)
      {
        m_opacity = 0.0;
      }
    }
    else
    {
      m_opacity = 1.0 - 0.75 * (1.0 - std::exp(exponent));
    }

    // downwards now holds the set of nodes whose centers are control points.
    // Use this to generate control points.
    controlPoints.reserve(downwards.size());
    if (downwards.size() <= 3)
    {
      lca = nullptr;
    }
    for (auto rit = downwards.rbegin(); rit != downwards.rend(); ++rit)
    {
      // If downwards has more than 3 control points, omit the LCA node.
      if (lca == *rit)
      {
        continue;
      }
      auto nodeRect = (*rit)->boundingRect();
      nodeRect = (*rit)->mapRectToScene(nodeRect);
      controlPoints.push_back(nodeRect.center());
    }

    // Adjust the control points by the "tightness" parameter beta.
    std::size_t nn = controlPoints.size();
    if (nn > 2)
    {
      double bt = gen ? gen->beta() : 0.9;
      QPointF pi = controlPoints[0];
      QPointF pf = controlPoints[nn - 1];
      for (std::size_t ii = 1; ii < nn - 1; ++ii)
      {
        controlPoints[ii] = bt * controlPoints[ii] + (1. - bt) * (pi + (ii / (nn - 1)) * (pf - pi));
      }
    }

#if SMTK_DBG_ARC
    std::cout << "Arc nodes\n";
    for (const auto& dne : downwards)
    {
      std::cout << "  " << dne->typeName();
      if (auto* foo = dynamic_cast<qtBaseObjectNode*>(dne))
      {
        std::cout << " " << foo->object()->name() << "\n";
      }
      else
      {
        std::cout << "\n";
      }
    }
#endif
  }

  constexpr std::size_t maxDegree = 3;
  std::size_t degree;
  auto knot = qtBSpline2D::uniformKnotVectorForInterpolatedEndpoints(
    controlPoints.size(), maxDegree, &degree);

  std::vector<QPointF> pathPoints;
  bool ok = qtBSpline2D::splineToCubicPath(degree, controlPoints, knot, pathPoints);
  (void)ok;

#if SMTK_DBG_ARC
  std::cout << "== " << pathPoints.size() << " points on path\n";
#endif

  m_computedPath.moveTo(pathPoints[0]);
  for (std::size_t ii = 1; ii < pathPoints.size();)
  {
    if (ii + 2 < pathPoints.size())
    {
#if SMTK_DBG_ARC
      std::cout << "c";
#endif
      m_computedPath.cubicTo(pathPoints[ii], pathPoints[ii + 1], pathPoints[ii + 2]);
      ii += 4;
    }
    else if (ii + 1 < pathPoints.size())
    {
#if SMTK_DBG_ARC
      std::cout << "q";
#endif
      m_computedPath.quadTo(pathPoints[ii], pathPoints[ii + 1]);
      ii += 3;
    }
    else
    {
#if SMTK_DBG_ARC
      std::cout << "l";
#endif
      m_computedPath.lineTo(pathPoints[ii]);
      ii += 2;
    }
  }
#if SMTK_DBG_ARC
  std::cout << "\n";
#endif
  this->setPath(m_computedPath);
  return 1;
}

void qtResourceDiagramArc::paint(
  QPainter* painter,
  const QStyleOptionGraphicsItem* option,
  QWidget* widget)
{
  (void)option;
  (void)widget;
  const auto& cfg(*this->scene()->configuration());

  QColor color = cfg.colorFromToken(m_arcType);
  qreal width = cfg.arcWidth();
  if (this->isSelected())
  {
    width *= 2;
    // NB. Selected arcs should be fully opaque.
  }
  else
  {
    // Unselected arcs vary in opacity.
    color.setAlphaF(m_opacity);
  }
  QPen pen;
  pen.setWidth(width);
  pen.setBrush(color);

  painter->strokePath(this->path(), pen);
}

} // namespace extension
} // namespace smtk
