//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskScene_h
#define smtk_extension_qtTaskScene_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/string/Token.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsScene>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtTaskArc;
class qtTaskEditor;
class qtBaseTaskNode;
class qtTaskViewConfiguration;

/**\brief A QGraphicsScene that holds workflow-related QGraphicsItems.
  *
  */
class SMTKQTEXT_EXPORT qtTaskScene : public QGraphicsScene
{
  Q_OBJECT

public:
  using Superclass = QGraphicsScene;

  qtTaskScene(qtTaskEditor*);
  ~qtTaskScene() override;

  /// Set/get the view configuration object passed to us from the parent widget.
  void setConfiguration(qtTaskViewConfiguration* config) { m_config = config; }
  qtTaskViewConfiguration* configuration() const { return m_config; }

  /// Return the parent task-editor.
  qtTaskEditor* editor() { return m_editor; }

public Q_SLOTS:
  /// Compute a layout of the \a nodes and \a arcs passed into this method.
  ///
  /// This uses graphviz to perform the layout and returns true on success.
  bool computeLayout(
    const std::unordered_set<qtBaseTaskNode*>& nodes,
    const std::unordered_set<qtTaskArc*>& arcs);

protected:
  /// Snaps the given \a x and \a y coordinate to the next available top left grid point.
  /// Optionally, the grid can be scaled with the \a resolution parameter.
  static QPointF snapToGrid(const qreal& x, const qreal& y, const qreal& resolution = 1.0);

  /// Draw a cross-hatched grid for the background.
  void drawBackground(QPainter* painter, const QRectF& rect) override;

  qtTaskViewConfiguration* m_config{ nullptr };
  qtTaskEditor* m_editor{ nullptr };
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtTaskScene_h
