//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtGroupingNode_h
#define smtk_extension_qtGroupingNode_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtBaseNode.h"

#include "smtk/common/UUID.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsItem>
#include <QGraphicsScene>

class QAbstractItemModel;
class QGraphicsTextItem;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtDiagramGenerator;
class GroupingNodeWidget;

/**\brief Rendering and interaction code for nodes that represent groups
  *       of resource and component nodes.
  *
  */
class SMTKQTEXT_EXPORT qtGroupingNode : public qtBaseNode
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem);
  Q_PROPERTY(QPointF pos READ pos WRITE setPos);
  Q_PROPERTY(qreal rotation READ rotation WRITE setRotation);

public:
  smtkSuperclassMacro(qtBaseNode);
  smtkTypeMacro(smtk::extension::qtGroupingNode);

  qtGroupingNode(
    qtDiagramGenerator* generator,
    smtk::string::Token groupName,
    QGraphicsItem* parent = nullptr);
  qtGroupingNode(
    const smtk::common::UUID& uid,
    const std::array<double, 2>& location,
    qtDiagramGenerator* generator,
    smtk::string::Token groupName,
    QGraphicsItem* parent = nullptr);
  ~qtGroupingNode() override = default;

  smtk::common::UUID nodeId() const override;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  /// Handle renames, etc.
  void dataUpdated() override;

  /// Return the group name string-token.
  smtk::string::Token groupName() const { return m_group; }

  /// Set/get a label to use for the node's name instead of its group.
  ///
  /// If no label is set, the label() method will return m_group.
  bool setLabel(smtk::string::Token label);
  smtk::string::Token label() const;

  /// Return the group name as the node's name.
  ///
  /// This will return m_label.data() if it is valid or m_group.data() otherwise.
  std::string name() const override;

protected:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  /// Handle pointer hovers
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

  /// Update the node bounds to fit its content.
  int updateSize() override;

  smtk::string::Token m_group;
  smtk::string::Token m_label;
  smtk::common::UUID m_id;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtGroupingNode_h
