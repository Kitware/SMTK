//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramView_h
#define smtk_extension_qtDiagramView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/PublicPointerDefs.h"

#include <QGraphicsView>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

class qtDiagram;
class qtDiagramScene;

/**\brief A widget that holds a Qt scene graph.
  *
  */
class SMTKQTEXT_EXPORT qtDiagramView : public QGraphicsView
{
  Q_OBJECT

public:
  using Superclass = QGraphicsView;

  qtDiagramView(qtDiagramScene* scene, qtDiagram* widget = nullptr);
  ~qtDiagramView() override;

  qtDiagram* diagram() const;

  /// Temporarily change modes until \a snapBackOnReleaseKey is released.
  ///
  /// When qtDiagramViewMode classes capture key presses (and **only** presses,
  /// not releases) in order to use modifier keys to switch modes temporarily,
  /// they can call this method on the view and, when the key is released by
  /// the user, the mode will revert from the \a snapToMode back to the mode
  /// before \a addModeSnapback was invoked.
  void addModeSnapback(Qt::Key snapBackOnReleaseKey, smtk::string::Token snapToMode);

protected:
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

  // Handle translate/scale on rubber-band mouse-motion
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

  // Handle worklets being dropped into this view.
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDiagramView_h
