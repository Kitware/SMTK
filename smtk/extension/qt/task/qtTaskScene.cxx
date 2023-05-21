//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskArc.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskNode.h"

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

namespace smtk
{
namespace extension
{

qtTaskScene::qtTaskScene(qtTaskEditor* parent)
  : Superclass(parent->widget())
  , m_editor(parent)
{
}

qtTaskScene::~qtTaskScene() = default;

bool qtTaskScene::computeLayout(
  const std::unordered_set<qtTaskNode*>& nodes,
  const std::unordered_set<qtTaskArc*>& arcs)
{
#if SMTK_ENABLE_GRAPHVIZ_SUPPORT
  // compute dot string
  qreal maxHeight = 0.0;
  qreal maxY = 0;
  std::string dotString;
  {
    std::stringstream nodeString;
    std::stringstream edgeString;

    for (const auto& node : nodes)
    {
      // Ignore hidden nodes
      if (!node->isVisible() || !node->task())
      {
        continue;
      }

      const QRectF& b = node->boundingRect();
      qreal width = b.width() / POINTS_PER_INCH; // convert from points to inches
      qreal height = b.height() / POINTS_PER_INCH;
      if (maxHeight < height)
      {
        maxHeight = height;
      }

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
      if (!node->isVisible() || !node->task())
      {
        continue;
      }

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

        maxY = std::max(maxY, y);
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
    int i = -2;
    for (const auto& node : nodes)
    {
      if (!node->isVisible() || !node->task())
      {
        continue;
      }
      i += 2;

      node->setPos(qtTaskScene::snapToGrid(coords[i], coords[i + 1]));
    }
  }

  return 1;
#else  // NodeEditor_ENABLE_GRAPHVIZ
  (void)nodes;
  (void)arcs;
  return false;
#endif // NodeEditor_ENABLE_GRAPHVIZ
}

QPointF qtTaskScene::snapToGrid(const qreal& x, const qreal& y, const qreal& resolution)
{
  // const auto gridSize = pqNodeEditorUtils::CONSTS::GRID_SIZE * resolution;
  const auto gridSize = 25 * resolution;
  return QPointF(x - std::fmod(x, gridSize), y - std::fmod(y, gridSize));
}

void qtTaskScene::drawBackground(QPainter* painter, const QRectF& rect)
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
  const QPointF& snappedTopLeft = qtTaskScene::snapToGrid(recL, recT, gridResolution);

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
