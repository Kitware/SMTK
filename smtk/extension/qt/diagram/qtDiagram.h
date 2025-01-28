//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagram_h
#define smtk_extension_qtDiagram_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/project/Project.h"
#include "smtk/task/Manager.h"
#include "smtk/view/Configuration.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeContainer.h"

#include "smtk/string/Token.h"

#include "smtk/PublicPointerDefs.h"

#include "nlohmann/json.hpp"

#include <QWidget>

#include <unordered_map>
#include <unordered_set>

class QLabel;
class QFrame;
class QToolBar;

namespace smtk
{
namespace extension
{

class qtBaseArc;
class qtBaseNode;
class qtDiagramGenerator;
class qtDiagramLegend;
class qtDiagramScene;
class qtDiagramView;
class qtDiagramViewMode;

/**\brief A widget that displays diagrams of SMTK resources.
  *
  * The purpose of this class is to (1) implement a qtBaseView
  * which provides a QGraphicsView/QGraphicsScene canvas and
  * (2) act as a clearing house for objects that manage items in
  * the scene which correspond to persistent objects.
  *
  * It owns
  * + A QGraphicsView and QGraphicsScene
  * + objects which inherit qtGraphicsMode and handle user
  *   interactions with the scene
  * + objects which inherit qtDiagramGenerator and make changes
  *   in the scene which reflect changes to persistent objects.
  *
  * The diagram is assumed to consist of
  * (1) nodes (which inherit qtBaseNode),
  * (2) arcs connecting nodes (which inherit qtBaseArc), and
  * (3) any other QGraphicsItem objects inserted by qtDiagramGenerator
  *     (but these must be managed by the generator; the diagram will
  *     not provide facilities to manage them).
  */
class SMTKQTEXT_EXPORT qtDiagram : public qtBaseView
{
  Q_OBJECT
  Q_PROPERTY(smtk::string::Token mode READ mode WRITE requestModeChange);

public:
  smtkTypenameMacro(smtk::extension::qtDiagram);
  smtkSuperclassMacro(smtk::extension::qtBaseView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtDiagram(const smtk::view::Information& info);
  ~qtDiagram() override;

  qtDiagramScene* diagramScene() const;
  qtDiagramView* diagramWidget() const;

  /// Return the legend widget held in the sidebar().
  qtDiagramLegend* legend() const;

  /// Return the top frame of the central area of the diagram.
  QFrame* topFrame() const;

  /// Provide the scene with a way to identify graphical items.
  ///
  /// Many items in the scene correspond to persistent objects
  /// and thus use their object's UUID. Others generate their
  /// own UUID.
  ///
  /// Note that generators should only insert nodes into this map
  /// that they wish other generators to reference (since only one
  /// object can exist in the map for a given UUID). It is possible
  /// for a multiple objects to represent the same UUID, but they
  /// cannot all be indexed by the diagram.
  qtBaseNode* findNode(const smtk::common::UUID& nodeId) const;

  /// Insert a node into the diagram's reverse-lookup map.
  bool addNode(qtBaseNode* node, bool enforceInteractionMode = true);

  /// Remove a node from the diagram.
  ///
  /// This will also remove all arcs attached to the node as well as it's children.
  bool removeNode(qtBaseNode* node);

  /// Remove a node index from the diagram's maps.
  ///
  bool removeNodeIndex(const smtk::common::UUID& nodeId);

  /// Insert an \a arc into the diagram's nodal lookup maps.
  ///
  /// The \a arc must be non-null and have non-null endpoint-nodes.
  bool addArc(qtBaseArc* arc, bool enforceSelectionMode = true);

  /// Remove an \a arc from the diagram's lookup maps.
  ///
  /// This requires the \a arc to have been properly indexed in order to be removed.
  /// If removing the arc causes empty entries in either the forward or reverse
  /// index, these are removed.
  bool removeArc(qtBaseArc* arc);

  /// Return a set of all the arcs incident to/from the \a node.
  std::unordered_set<qtBaseArc*> arcsOfNode(qtBaseNode* node);

  /// Return the set of all arcs *outgoing* from the \a node, indexed by the
  /// destination node.
  ///
  /// Contrast this method (which only returns *outgoing* arcs) to the arcsOfNode()
  /// method that returns both outgoing *and* incoming arcs.
  /// This method is faster than arcsOfNode().
  const std::unordered_map<
    qtBaseNode*,
    std::unordered_map<smtk::string::Token, std::unordered_set<qtBaseArc*>>>*
  arcsFromNode(qtBaseNode* node) const;

  /// Reverse arc lookup (i.e., find predecessor nodes attached to a given \a successor).
  std::unordered_set<qtBaseNode*> predecessorsOf(qtBaseNode* successor) const;

  /// Find arcs connecting an (ordered) pair of nodes.
  ///
  /// All arcs are considered directional and must have been created
  /// in the same \a source to \a target order as you provide nodes to this method.
  ///
  /// If you do not know the proper node order, make two calls to this method
  /// and examine both returned sets.
  ///
  /// This method simply calls arcsFromNode() and searches the result.
  const std::unordered_map<smtk::string::Token, std::unordered_set<qtBaseArc*>>* findArcs(
    qtBaseNode* source,
    qtBaseNode* target) const;

  /// The default configuration for the view.
  static std::shared_ptr<smtk::view::Configuration> defaultConfiguration();

  ///@{
  /// Set and get the current configuration of the view.
  ///
  /// This includes, for example, the location of persistent objects in the scene
  /// and items in the scene that do not correspond to a persistent object but
  /// which should nonetheless persist across file load/save.
  bool configure(const nlohmann::json& data);
  nlohmann::json configuration() const;
  ///@}

  /// Report the current user interaction mode.
  ///
  /// This will be one of: "pan", "select", "connect", or "disconnect"; but the list
  /// may be extended in the future.
  smtk::string::Token mode() const;

  /// Return the default mode for the editor.
  ///
  /// If a non-default mode wishes to allow users to "escape" from the mode
  /// (usually via the Escape key), they can request a change to this mode.
  /// The "connect" and "disconnect" modes currently use this method.
  smtk::string::Token defaultMode() const;

  /// Return the object managing the current mode.
  ///
  /// \sa mode
  qtDiagramViewMode* modeObject() const;

  /// Return the map from mode names to modes.
  const std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramViewMode>>& modes() const;

  /// Return a toolbar to which modes can add actions/widgets.
  ///
  /// Any actions a mode adds to the toolbar should be deleted or hidden when switching away from
  /// the mode.
  QToolBar* tools() const;

  /// Return the application state (smtk::common::Managers) object this editor was configured with.
  smtk::common::Managers::Ptr managers() const;

  /// Return the generators used to maintain the content of this diagram
  /// indexed by the name of the diagram content.
  const std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramGenerator>>& generators()
    const;

  /// Methods intended for diagram generators.
  //@{
  /// Hint to the diagram that the given rectangle (in scene coordinates) should be made visible
  /// in the view.
  ///
  /// Generally, the diagram will respond by taking the union of all rectangles provided by
  /// its generators and ensuring the result is visible. It will not zoom in but will zoom out
  /// if the resulting rectangle is not completely contained in the diagramWidget()'s active
  /// viewport.
  ///
  /// This method should be called by generators within their updateSceneArcs/Nodes() methods.
  /// After the diagram has invoked this method on all its generators, it will determine
  /// whether to make changes to the viewport.
  ///
  /// If different generators provide different \a priority values, the diagram will
  /// prefer the generator(s) whose \a priority is the highest.
  void includeInView(const QRectF& inclusion, int priority = 0);
  //@}

  /// True when nodes are enabled (i.e., users may interact with them) and false otherwise.
  bool nodesEnabled() const;
  /// True when nodes are selectable (i.e., users may select them) and false otherwise.
  bool nodeSelectionEnabled() const;
  /// True when arcs are selectable and false otherwise.
  bool arcSelectionEnabled() const;

  /// Return the sidebar widget.
  ///
  /// This method is exposed so that diagram generators can add widgets to the sidebar.
  QWidget* sidebar() const;

  /// Return the bounding rect of visible nodes and arcs in the scene.
  ///
  /// This only includes nodes and arcs, not other QGraphicsItems.
  QRectF visibleBounds() const;

Q_SIGNALS:
  /// Emitted by modeChangeRequested when the mode is actually changed
  /// (and not when unchanged).
  void modeChanged(smtk::string::Token nextMode);

public Q_SLOTS:
  /// Enable (or disable if \a shouldEnable is false) node interactivity.
  void enableNodes(bool shouldEnable);
  /// Enable (or disable if \a shouldEnable is false) node selectability.
  void enableNodeSelection(bool shouldEnable);
  /// Enable (or disable if \a shouldEnable is false) arc selection.
  void enableArcSelection(bool shouldEnable);
  /// Request a change in the user-interaction mode.
  virtual void requestModeChange(smtk::string::Token mode);
  /// Open/close the sidebar used to display information about nodes/arcs
  /// in the diagram.
  virtual void toggleSidebar(bool showSidebar);
  /// Called when the scene's Qt selection changes.
  ///
  /// This is used to send the relevant portion of the Qt selection to SMTK.
  /// It returns true when the SMTK selection was modified and false otherwise.
  virtual bool updateSMTKSelection();
  /// Called the SMTK's application-wide selection changes.
  ///
  /// This is used to update the diagram's selection to match any persistent
  /// objects represented as nodes or arcs in the graph.
  ///
  /// Note that this method will always overwrite the Qt selection; it does
  /// not verify whether the change to the SMTK selection was caused by
  /// this object. (That check is performed before this method is invoked.)
  virtual void updateQtSelection();

protected Q_SLOTS:
  /// Invoked when a user clicks on a title-bar button to change modes.
  void modeChangeRequested(QAction* modeAction);
  /// Invoked when a user moves a node.
  void onNodeGeometryChanged();

protected:
  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtDiagram_h
