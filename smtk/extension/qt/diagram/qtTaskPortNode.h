//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskPortNode_h
#define smtk_extension_qtTaskPortNode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainterPath>

class QAbstractItemModel;
class QGraphicsPathItem;
class QGraphicsTextItem;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace resource
{
class PersistentObject;
}
namespace task
{
class Port;
}
namespace extension
{

/**\brief Rendering and interaction code for nodes that represent Ports of Tasks.
  *
  */
class SMTKQTEXT_EXPORT qtTaskPortNode : public qtBaseObjectNode
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem);
  Q_PROPERTY(QPointF pos READ pos WRITE setPos);
  Q_PROPERTY(qreal rotation READ rotation WRITE setRotation);

public:
  smtkSuperclassMacro(qtBaseObjectNode);
  smtkTypeMacro(smtk::extension::qtTaskPortNode);

  qtTaskPortNode(
    qtDiagramGenerator* generator,
    smtk::resource::PersistentObject* resource,
    QGraphicsItem* parent = nullptr);
  ~qtTaskPortNode() override = default;

  smtk::common::UUID nodeId() const override;
  smtk::resource::PersistentObject* object() const override;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  smtk::task::Port* port() const { return m_port; }

  void setLength(qreal newLength);
  void setAngle(qreal newAngle);

protected:
  friend class PortNodeWidget;
  void updateToolTip();
  // Adjusts the orientation of the port based on a location
  // with respects to its parent task
  void adjustOrientation(const QPointF& pnt);
  // Adjust the position of the Port node if snapping is
  // requested.
  void adjustPosition(QPointF& pnt);
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& val) override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Handle pointer hovers
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
  void updateArc();
  void updateShape();

  qreal m_length;
  qreal m_angle;
  QPainterPath m_path;
  QGraphicsPathItem* m_arc;

  smtk::task::Port* m_port{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskPortNode_h
