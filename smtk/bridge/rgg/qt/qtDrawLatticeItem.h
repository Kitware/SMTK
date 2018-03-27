//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDrawLatticeItem - Graphics item for RGG schema planner
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_qtDrawLatticeItem_h
#define __smtk_bridge_rgg_qt_qtDrawLatticeItem_h

#include "smtk/bridge/rgg/qt/Exports.h"
#include "smtk/bridge/rgg/qt/qtLatticeHelper.h"

#include <QGraphicsPolygonItem>
#include <QObject>

namespace smtk
{
namespace model
{
class EntityRef;
}
}

class qtDrawLatticeItem : public QGraphicsPolygonItem
{
  typedef QGraphicsPolygonItem Superclass;

public:
  // Rect: layer expands horizontally and cellIdx expands vertically
  // Hex: layer expands along radius and cellIdx expands along the edges
  qtDrawLatticeItem(const QPolygonF& polygon, int layer, int cellIdx, qtCellReference const& ref,
    QGraphicsItem* parent = 0);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  QString text() const;

  int layer();
  int cellIndex();

  smtk::model::EntityRef getPart();

  void select() { this->m_refCell.setDrawMode(qtCellReference::DrawMode::SELECTED); }

  bool checkRadiusIsOk(double r) const { return !this->m_refCell.radiusConflicts(r); }

  QPointF getCentroid() const;

protected:
  void drawText(QPainter* painter);

private:
  qtCellReference const& m_refCell;
  int m_layer;
  int m_cellIndex;
  QPointF m_localCenter;
};

#endif
