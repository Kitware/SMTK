//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtBaseArc.h"
#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"

#include "smtk/Options.h"
#include "smtk/io/Logger.h"

#include <QApplication>
#include <QPainter>

#if SMTK_ENABLE_GRAPHVIZ_SUPPORT
// Older Graphviz releases (before 2.40.0) require `HAVE_CONFIG_H` to define
// `POINTS_PER_INCH`.
#define HAVE_CONFIG_H
#include <cgraph.h>
#include <geom.h>
#include <gvc.h>
#undef HAVE_CONFIG_H
#endif // SMTK_ENABLE_GRAPHVIZ_SUPPORT

#include <cmath>

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtDiagramScene::qtDiagramScene(qtDiagram* parent)
  : Superclass(parent->widget())
  , m_diagram(parent)
{
}

qtDiagramScene::~qtDiagramScene() = default;

std::unordered_set<qtBaseNode*> qtDiagramScene::nodesOfSelection() const
{
  std::unordered_set<qtBaseNode*> result;
  for (const auto& item : this->selectedItems())
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      result.insert(node);
    }
  }
  return result;
}

bool qtDiagramScene::computeLayout(
  const std::unordered_set<qtBaseNode*>& nodes,
  const std::unordered_set<qtBaseArc*>& arcs)
{
#if SMTK_ENABLE_GRAPHVIZ_SUPPORT
  // compute dot string
  QPointF oldCenter;
  std::string dotString;
  {
    std::stringstream nodeString;
    std::stringstream edgeString;

    double scale = 1. / nodes.size();
    for (const auto& node : nodes)
    {
      // Ignore hidden nodes
      // if (!node->isVisible() || !node->nodeId())
      // {
      //   continue;
      // }

      const QRectF& b = node->sceneBoundingRect();
      qreal width = b.width() / POINTS_PER_INCH; // convert from points to inches
      qreal height = b.height() / POINTS_PER_INCH;
      oldCenter += scale * node->pos();

      // Construct the string declaring a node.
      // See https://www.graphviz.org/pdf/libguide.pdf for more detail
      nodeString << "n" << node << "["
                 << "shape=record,"
                 << "width=" << width << ","
                 << "height=" << height << ""
                 << "];\n";
    }

    // Construct the string representing all arcs in the graph
    // See https://www.graphviz.org/pdf/libguide.pdf for more detail
    for (const auto& arc : arcs)
    {
      edgeString << "n" << arc->predecessor() << " -> "
                 << "n" << arc->successor() << ";\n";
    }

    // describe the overall look of the graph. For example : rankdir=LR -> Left To Right layout
    // See https://www.graphviz.org/pdf/libguide.pdf for more detail
    dotString += "digraph g {\nrankdir=TB;splines = line;graph[pad=\"0\", ranksep=\"0.6\", "
                 "nodesep=\"0.6\"];\n" +
      nodeString.str() + edgeString.str() + "\n}";
  }

  std::vector<qreal> coords(2 * nodes.size(), 0.0);
  // compute layout
  {
    Agraph_t* G = agmemread(dotString.data());
    GVC_t* gvc = gvContext();
    if (!G || !gvc || gvLayout(gvc, G, "dot"))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "[NodeEditorPlugin] Cannot intialize Graphviz context.");
      return 0;
    }

    // read layout
    int i = -2;
    for (const auto& node : nodes)
    {
      // if (!node->isVisible() || !node->nodeId())
      // {
      //   continue;
      // }

      i += 2;

      std::ostringstream nodeName;
      nodeName << "n" << node;

      Agnode_t* n = agnode(G, const_cast<char*>(nodeName.str().c_str()), 0);
      if (n != nullptr)
      {
        const auto& coord = ND_coord(n);
        const auto& w = ND_width(n);
        const auto& h = ND_height(n);

        auto& x = coords[i];
        auto& y = coords[i + 1];
        x = (coord.x - w * POINTS_PER_INCH / 2.0); // convert w/h in inches to points
        y = (-coord.y - h * POINTS_PER_INCH / 2.0);
      }
    }

    // free memory
    int status = gvFreeLayout(gvc, G);
    status += agclose(G);
    status += gvFreeContext(gvc);
    if (status)
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(), "[NodeEditorPlugin] Error when freeing Graphviz resources.");
    }
  }

  // set positions
  {
    auto nc = static_cast<int>(coords.size() / 2);
    double scale = 1. / nc;
    QPointF newCenter;
    for (int ii = 0; ii < nc; ++ii)
    {
      newCenter += QPointF(coords[2 * ii], coords[2 * ii + 1]) * scale;
    }
    oldCenter -= newCenter;
    int i = -2;
    for (const auto& node : nodes)
    {
      // if (!node->isVisible() || !node->nodeId())
      // {
      //   continue;
      // }
      i += 2;

      node->setPos(
        qtDiagramScene::snapToGrid(coords[i] + oldCenter.x(), coords[i + 1] + oldCenter.y()));
    }
  }

  return 1;
#else  // NodeEditor_ENABLE_GRAPHVIZ
  (void)nodes;
  (void)arcs;
  return false;
#endif // NodeEditor_ENABLE_GRAPHVIZ
}

void qtDiagramScene::alignHorizontal(const std::unordered_set<qtBaseNode*>& nodeSet, int alignment)
{
  // WARNING: If you make changes to this function, also change alignVertical().
  // The two functions are identical except for several calls specific to x- vs. y-axis
  // coordinate methods on various objects.

  // Create a map from selected nodes to their current coordinates.
  // Depending on \a alignment, the map values are the left-, center-, or right-most
  // point of their bounds in the scene.
  // This also computes the minimum/maximum coordinates across the nodes.
  std::map<qtBaseNode*, qreal> nodes;
  qreal xlo = std::numeric_limits<qreal>::infinity();
  qreal xhi = -std::numeric_limits<qreal>::infinity();
  for (const auto& item : nodeSet)
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      auto rect = node->sceneBoundingRect();
      qreal itemX =
        (alignment < 0 ? rect.left() : (alignment > 0 ? rect.right() : rect.center().x()));
      nodes[node] = itemX;
      if (rect.left() < xlo)
      {
        xlo = rect.left();
      }
      if (rect.right() > xhi)
      {
        xhi = rect.right();
      }
    }
  }
  // Compute the coordinate to align the nodes to (\a location):
  qreal location = (alignment < 0 ? xlo : (alignment > 0 ? xhi : (xlo + xhi) / 2.));
  // Iterate over the set of nodes and update each node's coordinate
  // to match \a location, taking parent-child coordinate systems into account.
  for (const auto& entry : nodes)
  {
    qreal delta = location - entry.second;
    QPointF pt = entry.first->pos();
    QGraphicsItem* pp = entry.first->parentItem();
    while (pp)
    {
      if (auto* parent = dynamic_cast<qtBaseNode*>(pp))
      {
        auto pit = nodes.find(parent);
        if (pit != nodes.end())
        {
          delta -= location - pit->second;
        }
      }
      pp = pp->parentItem();
    }
    if (delta != 0.)
    {
      pt.setX(pt.x() + delta);
      entry.first->setPos(pt);
    }
  }
}

void qtDiagramScene::alignVertical(const std::unordered_set<qtBaseNode*>& nodeSet, int alignment)
{
  // WARNING: If you make changes to this function, also change alignHorizontal().
  // The two functions are identical except for several calls specific to x- vs. y-axis
  // coordinate methods on various objects.

  // Create a map from selected nodes to their current coordinates.
  // Depending on \a alignment, the map values are the top-, center-, or bottom-most
  // point of their bounds in the scene.
  // This also computes the minimum/maximum coordinates across the nodes.
  std::map<qtBaseNode*, qreal> nodes;
  qreal ylo = std::numeric_limits<qreal>::infinity();
  qreal yhi = -std::numeric_limits<qreal>::infinity();
  for (const auto& item : nodeSet)
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      auto rect = node->sceneBoundingRect();
      qreal itemY =
        (alignment < 0 ? rect.top() : (alignment > 0 ? rect.bottom() : rect.center().y()));
      nodes[node] = itemY;
      if (rect.top() < ylo)
      {
        ylo = rect.top();
      }
      if (rect.bottom() > yhi)
      {
        yhi = rect.bottom();
      }
    }
  }
  // Compute the coordinate to align the nodes to (\a location):
  qreal location = (alignment < 0 ? ylo : (alignment > 0 ? yhi : (ylo + yhi) / 2.));
  // Iterate over the set of nodes and update each node's coordinate
  // to match \a location, taking parent-child coordinate systems into account.
  for (const auto& entry : nodes)
  {
    qreal delta = location - entry.second;
    QPointF pt = entry.first->pos();
    QGraphicsItem* pp = entry.first->parentItem();
    while (pp)
    {
      if (auto* parent = dynamic_cast<qtBaseNode*>(pp))
      {
        auto pit = nodes.find(parent);
        if (pit != nodes.end())
        {
          delta -= location - pit->second;
        }
      }
      pp = pp->parentItem();
    }
    if (delta != 0.)
    {
      pt.setY(pt.y() + delta);
      entry.first->setPos(pt);
    }
  }
}

void qtDiagramScene::distributeHorizontal(
  const std::unordered_set<qtBaseNode*>& nodeSet,
  smtk::string::Token distribution)
{
  // WARNING: If you make changes to this function, also change alignVertical().
  // The two functions are identical except for several calls specific to x- vs. y-axis
  // coordinate methods on various objects.
  std::map<qtBaseNode*, qreal> nodes;
  std::multimap<qreal, qtBaseNode*> order;
  qreal xlo = std::numeric_limits<qreal>::infinity();
  qreal xhi = -std::numeric_limits<qreal>::infinity();
  // cumSize holds the accumulated size (width) of items, which is used
  // when distribution == "gaps"_token to determine how much additional
  // space remains that can be distributed between items.
  qreal cumSize = 0.;
  for (const auto& item : nodeSet)
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      auto rect = node->sceneBoundingRect();
      qreal itemX = rect.center().x();
      nodes[node] = itemX;
      order.insert(std::make_pair(itemX, node));
      cumSize += rect.width();
      if (distribution == "gaps"_token)
      {
        if (rect.left() < xlo)
        {
          xlo = rect.left();
        }
        if (rect.right() > xhi)
        {
          xhi = rect.right();
        }
      }
      else if (distribution == "centers"_token)
      {
        if (itemX < xlo)
        {
          xlo = itemX;
        }
        if (itemX > xhi)
        {
          xhi = itemX;
        }
      }
    }
  }
  // Now compute total (unadjusted for parents) movement of each node.
  // For "gaps", spacing is the distance between node bounding rectangles.
  // For "centers", spacing is the distance between node centers.
  // Note that spacing can be any real number (positive, zero, or negative).
  qreal spacing =
    (distribution == "centers"_token ? (xhi - xlo) / (nodes.size() - 1.)
                                     : (xhi - xlo - cumSize) / (nodes.size() - 1.));
  qreal xx = xlo; // Distribute from left to right, starting with xlo.
  // Assign each node an initial "delta" (horizontal distance to move) by accumulating
  // spacing from the far left item's node-center (xlo) coordinate:
  for (const auto& entry : order)
  {
    qreal init = entry.first;
    qreal currHalfWidth = entry.second->sceneBoundingRect().width() / 2.;
    if (distribution == "gaps"_token)
    {
      xx += currHalfWidth;
    }
    // Store the difference between the initial location (init) of
    // the item's center and the final position (xx).
    nodes[entry.second] = xx - init;
    xx += (distribution == "centers"_token ? spacing : currHalfWidth + spacing);
  }
  // Now compute the actual "delta" for each item by accounting for parent-child
  // relationships, then assign the new location.
  for (const auto& entry : order)
  {
    QPointF pt = entry.second->pos();
    qreal px = pt.x() + nodes[entry.second];
    auto* pp = entry.second->parentItem();
    while (pp)
    {
      if (auto* nn = dynamic_cast<qtBaseNode*>(pp))
      {
        auto pit = nodes.find(nn);
        if (pit != nodes.end())
        {
          px -= nodes[nn];
        }
      }
      pp = pp->parentItem();
    }
    pt.setX(px);
    entry.second->setPos(pt);
  }
}

void qtDiagramScene::distributeVertical(
  const std::unordered_set<qtBaseNode*>& nodeSet,
  smtk::string::Token distribution)
{
  // WARNING: If you make changes to this function, also change alignHorizontal().
  // The two functions are identical except for several calls specific to x- vs. y-axis
  // coordinate methods on various objects.
  std::map<qtBaseNode*, qreal> nodes;
  std::multimap<qreal, qtBaseNode*> order;
  qreal ylo = std::numeric_limits<qreal>::infinity();
  qreal yhi = -std::numeric_limits<qreal>::infinity();
  // cumSize holds the accumulated size (height) of items, which is used
  // when distribution == "gaps"_token to determine how much additional
  // space remains that can be distributed between items.
  qreal cumSize = 0.;
  for (const auto& item : nodeSet)
  {
    if (auto* node = dynamic_cast<qtBaseNode*>(item))
    {
      auto rect = node->sceneBoundingRect();
      qreal itemY = rect.center().y();
      nodes[node] = itemY;
      order.insert(std::make_pair(itemY, node));
      cumSize += rect.height();
      if (distribution == "gaps"_token)
      {
        if (rect.left() < ylo)
        {
          ylo = rect.left();
        }
        if (rect.right() > yhi)
        {
          yhi = rect.right();
        }
      }
      else if (distribution == "centers"_token)
      {
        if (itemY < ylo)
        {
          ylo = itemY;
        }
        if (itemY > yhi)
        {
          yhi = itemY;
        }
      }
    }
  }
  // Now compute total (unadjusted for parents) movement of each node.
  // For "gaps", spacing is the distance between node bounding rectangles.
  // For "centers", spacing is the distance between node centers.
  // Note that spacing can be any real number (positive, zero, or negative).
  qreal spacing =
    (distribution == "centers"_token ? (yhi - ylo) / (nodes.size() - 1.)
                                     : (yhi - ylo - cumSize) / (nodes.size() - 1.));
  qreal yy = ylo; // Distribute from left to right, starting with ylo.
  // Assign each node an initial "delta" (horizontal distance to move) by accumulating
  // spacing from the far left item's node-center (ylo) coordinate:
  for (const auto& entry : order)
  {
    qreal init = entry.first;
    qreal currHalfWidth = entry.second->sceneBoundingRect().height() / 2.;
    if (distribution == "gaps"_token)
    {
      yy += currHalfWidth;
    }
    // Store the difference between the initial location (init) of
    // the item's center and the final position (yy).
    nodes[entry.second] = yy - init;
    yy += (distribution == "centers"_token ? spacing : currHalfWidth + spacing);
  }
  // Now compute the actual "delta" for each item by accounting for parent-child
  // relationships, then assign the new location.
  for (const auto& entry : order)
  {
    QPointF pt = entry.second->pos();
    qreal py = pt.y() + nodes[entry.second];
    auto* pp = entry.second->parentItem();
    while (pp)
    {
      if (auto* nn = dynamic_cast<qtBaseNode*>(pp))
      {
        auto pit = nodes.find(nn);
        if (pit != nodes.end())
        {
          py -= nodes[nn];
        }
      }
      pp = pp->parentItem();
    }
    pt.setY(py);
    entry.second->setPos(pt);
  }
}

QPointF qtDiagramScene::snapToGrid(const qreal& x, const qreal& y, const qreal& resolution)
{
  // const auto gridSize = pqNodeEditorUtils::CONSTS::GRID_SIZE * resolution;
  const auto gridSize = 25 * resolution;
  return QPointF(x - std::fmod(x, gridSize), y - std::fmod(y, gridSize));
}

void qtDiagramScene::drawBackground(QPainter* painter, const QRectF& rect)
{
  // painter->setPen(pqNodeEditorUtils::CONSTS::COLOR_GRID);
  painter->setPen(QApplication::palette().mid().color());

  // get rectangle bounds
  const qreal recL = rect.left();
  const qreal recR = rect.right();
  const qreal recT = rect.top();
  const qreal recB = rect.bottom();

  // determine whether to use low or high resoltion grid
  const qreal gridResolution = (recB - recT) > 2000 ? 4 : 1;
  // const qreal gridSize = gridResolution * pqNodeEditorUtils::CONSTS::GRID_SIZE;
  const qreal gridSize = gridResolution * 25;

  // find top left corner of active rectangle and snap to grid
  // const QPointF& snappedTopLeft = pqNodeEditorScene::snapToGrid(recL, recT, gridResolution);
  const QPointF& snappedTopLeft = qtDiagramScene::snapToGrid(recL, recT, gridResolution);

  // iterate over the x range of the rectangle to draw vertical lines
  for (qreal x = snappedTopLeft.x(); x < recR; x += gridSize)
  {
    painter->drawLine(x, recT, x, recB);
  }

  // iterate over the y range of the rectangle to draw horizontal lines
  for (qreal y = snappedTopLeft.y(); y < recB; y += gridSize)
  {
    painter->drawLine(recL, y, recR, y);
  }
}

} // namespace extension
} // namespace smtk
