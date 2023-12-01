//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtBaseArc_h
#define smtk_extension_qtBaseArc_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>

namespace smtk
{
namespace extension
{

class qtDiagramGenerator;
class qtDiagramScene;
class qtBaseNode;

/**\brief A graphics item that represents an arc between nodes in a diagram.
  *
  */
class SMTKQTEXT_EXPORT qtBaseArc
  : public QObject
  , public QGraphicsPathItem
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QGraphicsPathItem);
  smtkTypeMacroBase(smtk::extension::qtBaseArc);

  using ArcType = smtk::string::Token;

  /// Construct an arc.
  ///
  /// By default, arcs monitor their predecessor and successor nodes; when either
  /// is moved, the pure-virtual updateArcPoints() is called and the arc geometry
  /// is updated.
  ///
  /// Note that because qtBaseArc has pure virtual methods, your subclass
  /// must call the following in its constructor (assuming your subclass provides
  /// implementations of pure virtual methods).
  ///
  /// ```cpp
  /// this->updateArcPoints();
  /// this->scene()->addItem(this);
  /// ```
  ///
  /// Arcs have caching turned off since they are cheap to draw and often have large
  /// bounding boxes that are mostly empty.
  qtBaseArc(
    qtDiagramGenerator* generator,
    qtBaseNode* predecessor,
    qtBaseNode* successor,
    ArcType arcType,
    QGraphicsItem* parent = nullptr);
  ~qtBaseArc() override;

  /// Get the bounding box of the node, which includes the border width and the label.
  QRectF boundingRect() const override;

  qtDiagramGenerator* generator() const { return m_generator; }
  qtDiagramScene* scene() const;
  ArcType arcType() const { return m_arcType; }
  qtBaseNode* predecessor() const { return m_predecessor; }
  qtBaseNode* successor() const { return m_successor; }

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)

  /// Recompute points specifying the shape of the arc based on the current
  /// endpoint-node positions and geometry.
  virtual int updateArcPoints() = 0;

protected:
  /// Draw the arc into the scene.
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

  qtDiagramGenerator* m_generator{ nullptr };
  qtBaseNode* m_predecessor{ nullptr };
  qtBaseNode* m_successor{ nullptr };
  ArcType m_arcType;
  QPainterPath m_computedPath;
  QPainterPath m_arrowPath;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtBaseArc_h
