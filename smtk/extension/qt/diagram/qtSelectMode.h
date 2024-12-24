//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtSelectMode_h
#define smtk_extension_qtSelectMode_h

#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

#include "smtk/view/SelectionObserver.h"

namespace smtk
{
namespace extension
{

class qtDiagramView;

/**\brief A mode where click+dragging on the background rubber-band selects nodes.
  *
  * This mode does not select arcs. \sa qtDisconnectMode to select arcs.
  */
class SMTKQTEXT_EXPORT qtSelectMode : public qtDiagramViewMode
{
  Q_OBJECT

public:
  using Superclass = qtDiagramViewMode;

  /// Construct a mode for \a diagram and add it to \a toolbar and \a modeGroup.
  qtSelectMode(qtDiagram* diagram, qtDiagramView* view, QToolBar* toolbar, QActionGroup* modeGroup);
  ~qtSelectMode() override;

public Q_SLOTS:
  /// Horizontally align selected nodes.
  virtual void alignLeft();
  virtual void alignHCenter();
  virtual void alignRight();

  /// Vertically align selected nodes.
  virtual void alignTop();
  virtual void alignVCenter();
  virtual void alignBottom();

  /// Distribute selected nodes horizontally.
  virtual void distributeHCenters();
  virtual void distributeHGaps();

  /// Distribute selected nodes vertically.
  virtual void distributeVCenters();
  virtual void distributeVGaps();

  /// Use graphviz to lay out selected nodes based on arcs between them (if built with graphviz).
  virtual void layoutSelectedNodesWithTheirArcs();

  /// Called when the set of selected nodes changes.
  virtual void enableSelectionSensitiveActions();

protected:
  /// Overriden to capture key presses that modulate modes and delete nodes.
  bool eventFilter(QObject* obj, QEvent* event) override;

  // void sceneCleared() override;
  void enterMode() override;
  void exitMode() override;

  /// If alignment/distribution actions do not exist, create them.
  void addModeButtons();
  /// Called when this mode is entered/exited to show/hide alignment/distribution actions.
  void showModeButtons(bool show = true);
  /// Called when the SMTK selection is updated to enable/disable actions that require a selection.
  void changeSelectionSensitiveActions(bool enable);

  QAction* m_alignLeft{ nullptr };
  QAction* m_alignHCenter{ nullptr };
  QAction* m_alignRight{ nullptr };
  QAction* m_alignTop{ nullptr };
  QAction* m_alignVCenter{ nullptr };
  QAction* m_alignBottom{ nullptr };

  QAction* m_distributeHCenters{ nullptr };
  QAction* m_distributeHGaps{ nullptr };
  QAction* m_distributeVCenters{ nullptr };
  QAction* m_distributeVGaps{ nullptr };

  QAction* m_relayoutNodes{ nullptr };

  QAction* m_resetViewportToBounds{ nullptr };

  smtk::view::SelectionObservers::Key m_selectionObserver;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtSelectMode_h
