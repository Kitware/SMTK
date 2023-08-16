//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskArc_h
#define smtk_extension_qtTaskArc_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>

namespace smtk
{
namespace task
{
class Adaptor;
}

namespace extension
{

class qtTaskEditor;
class qtTaskScene;
class qtBaseTaskNode;

/**\brief A widget that holds a Qt scene graph.
  *
  */
class SMTKQTEXT_EXPORT qtTaskArc
  : public QObject
  , public QGraphicsPathItem
{
  Q_OBJECT

public:
  using Superclass = QGraphicsPathItem;

  /// Arcs between nodes indicate a required ordering of tasks.
  enum class ArcType : int
  {
    Dependency, //!< Tasks are administratively forced to occur in order.
    Adaptor     //!< Tasks are technically forced into order by information flow.
  };

  qtTaskArc(
    qtTaskScene* scene,
    qtBaseTaskNode* predecessor,
    qtBaseTaskNode* successor,
    ArcType type = ArcType::Dependency,
    QGraphicsItem* parent = nullptr);
  qtTaskArc(
    qtTaskScene* scene,
    qtBaseTaskNode* predecessor,
    qtBaseTaskNode* successor,
    smtk::task::Adaptor* adaptor,
    QGraphicsItem* parent = nullptr);
  ~qtTaskArc() override;

  qtTaskScene* scene() const { return m_scene; }
  ArcType arcType() const { return m_arcType; }
  qtBaseTaskNode* predecessor() const { return m_predecessor; }
  qtBaseTaskNode* successor() const { return m_successor; }
  smtk::task::Adaptor* adaptor() const { return m_adaptor; }

  static ArcType arcTypeEnum(const std::string& enumerant, bool* match = nullptr);
  static std::string arcTypeName(ArcType enumerant);

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  int updateArcPoints();

protected:
  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;
  /// Draw the arc into the scene.
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  qtTaskScene* m_scene{ nullptr };
  qtBaseTaskNode* m_predecessor{ nullptr };
  qtBaseTaskNode* m_successor{ nullptr };
  smtk::task::Adaptor* m_adaptor{ nullptr }; // Only set when ArcType == Adaptor.
  ArcType m_arcType{ ArcType::Dependency };
  QPainterPath m_computedPath;
  QPainterPath m_arrowPath;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskArc_h
