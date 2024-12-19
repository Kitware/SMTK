//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramScene_h
#define smtk_extension_qtDiagramScene_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/string/Token.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsScene>

#include <unordered_set>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtBaseArc;
class qtDiagram;
class qtBaseNode;
class qtDiagramViewConfiguration;

/**\brief A QGraphicsScene that holds QGraphicsItems for diagrams.
  *
  * Diagrams are node+arc schematic figures whose items are maintained
  * by qtDiagramGenerator instances.
  */
class SMTKQTEXT_EXPORT qtDiagramScene : public QGraphicsScene
{
  Q_OBJECT

public:
  using Superclass = QGraphicsScene;

  qtDiagramScene(qtDiagram*);
  ~qtDiagramScene() override;

  /// Set/get the view configuration object passed to us from the parent widget.
  void setConfiguration(qtDiagramViewConfiguration* config) { m_config = config; }
  qtDiagramViewConfiguration* configuration() const { return m_config; }

  /// Return the parent diagram.
  qtDiagram* diagram() { return m_diagram; }

  std::unordered_set<qtBaseNode*> nodesOfSelection() const;

public Q_SLOTS:
  /// Compute a layout of the \a nodes and \a arcs passed into this method.
  ///
  /// This uses graphviz to perform the layout and returns true on success.
  bool computeLayout(
    const std::unordered_set<qtBaseNode*>& nodes,
    const std::unordered_set<qtBaseArc*>& arcs);

  /// These methods are called with +1, 0, or -1 from the above methods
  /// (alignLeft/HCenterRight and alignTop/VCenter/Bottom).
  /// Call these methods with \a aignment set to +1, 0, or -1
  /// to align nodes horizontally or vertically.
  ///
  /// When called with +1, nodes are aligned to their right-/top-most edges.
  /// When called with  0, nodes are aligned to their centers.
  /// When called with -1, nodes are aligned to their left-/bottom-most edges.
  virtual void alignHorizontal(const std::unordered_set<qtBaseNode*>& nodeSet, int alignment);
  virtual void alignVertical(const std::unordered_set<qtBaseNode*>& nodeSet, int alignment);

  /// Call these methods with \a distribution set to "centers" or "gaps"
  /// to redistribute nodes horizontally or vertically.
  ///
  /// When called with "centers", node center points are evenly distributed.
  /// When called with "gaps", the space between nodes is evenly distributed.
  virtual void distributeHorizontal(
    const std::unordered_set<qtBaseNode*>& nodeSet,
    smtk::string::Token distribution);
  virtual void distributeVertical(
    const std::unordered_set<qtBaseNode*>& nodeSet,
    smtk::string::Token distribution);

protected:
  /// Snaps the given \a x and \a y coordinate to the next available top left grid point.
  /// Optionally, the grid can be scaled with the \a resolution parameter.
  static QPointF snapToGrid(const qreal& x, const qreal& y, const qreal& resolution = 1.0);

  /// Draw a cross-hatched grid for the background.
  void drawBackground(QPainter* painter, const QRectF& rect) override;

  qtDiagramViewConfiguration* m_config{ nullptr };
  qtDiagram* m_diagram{ nullptr };
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtDiagramScene_h
