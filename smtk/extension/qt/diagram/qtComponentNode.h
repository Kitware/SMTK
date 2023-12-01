//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtComponentNode_h
#define smtk_extension_qtComponentNode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsItem>
#include <QGraphicsScene>

class QAbstractItemModel;
class QGraphicsTextItem;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace resource
{
class PersistentObject;
}
namespace extension
{

class ComponentNodeWidget;

/**\brief Rendering and interaction code for nodes that represent components of resources.
  *
  */
class SMTKQTEXT_EXPORT qtComponentNode : public qtBaseObjectNode
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem);
  Q_PROPERTY(QPointF pos READ pos WRITE setPos);
  Q_PROPERTY(qreal rotation READ rotation WRITE setRotation);

public:
  smtkSuperclassMacro(qtBaseObjectNode);
  smtkTypeMacro(smtk::extension::qtComponentNode);

  qtComponentNode(
    qtDiagramGenerator* generator,
    smtk::resource::PersistentObject* resource,
    QGraphicsItem* parent = nullptr);
  ~qtComponentNode() override = default;

  smtk::common::UUID nodeId() const override;
  smtk::resource::PersistentObject* object() const override;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  /// Handle renames, etc.
  void dataUpdated() override;

  smtk::resource::Component* component() const { return m_component; }

protected:
  friend class ComponentNodeWidget;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Handle pointer hovers
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

  smtk::resource::Component* m_component{ nullptr };
  ComponentNodeWidget* m_container{ nullptr };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtComponentNode_h
