//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtResourceDiagram_h
#define smtk_extension_qtResourceDiagram_h

#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QObject>

#include <unordered_map>
#include <unordered_set>

namespace smtk
{
namespace extension
{

class qtBaseArc;
class qtBaseNode;
class qtDiagram;
class qtResourceDiagramSummary;
class qtDiagramLegendEntry;

/**\brief A base class that maintains a diagram based on the results of operations which
  *       modify the state of the diagram's objective.
  */
class SMTKQTEXT_EXPORT qtResourceDiagram : public qtDiagramGenerator
{
  Q_OBJECT
public:
  smtkTypenameMacro(smtk::extension::qtResourceDiagram);
  smtkSuperclassMacro(qtDiagramGenerator);

  /// A collection of component-filters applied to a resource to identify objects to include/exclude.
  ///
  /// A flag is also provided to indicate whether the resource should have a node in the diagram.
  struct ResourceRules
  {
    /// Accept/reject matching resources as nodes in the diagram?
    bool resourceNode{ true };
    /// Rules to accept/reject components of matching resources.
    std::unordered_set<smtk::string::Token> componentRules;
  };
  /// A map from resource type-names to rules for accepting/rejecting persistent objects.
  using ObjectRules = std::unordered_map<smtk::string::Token, ResourceRules>;

  /// A comparator used to order leaf nodes around the diagram's circumference.
  struct VisualComparator
  {
    bool operator()(const std::vector<qtBaseNode*>& aa, const std::vector<qtBaseNode*>& bb) const;
    bool nodeLessThan(const qtBaseNode* cc, const qtBaseNode* dd) const;
  };

  using ArcLegendEntries = std::unordered_map<smtk::string::Token, qtDiagramLegendEntry*>;

  /// Type-aliases for the containers that represent the diagram's layout.
  //@{
  /// A radial "edge" of nodes starting at the diagram root and moving outward.
  using NodeEdge = std::vector<qtBaseNode*>;
  /// A circumferential ordering of node "edges"
  using NodeTree = std::set<NodeEdge, VisualComparator>;
  //@}

  qtResourceDiagram(
    const smtk::view::Information& info,
    const smtk::view::Configuration::Component& config,
    qtDiagram* parent);
  ~qtResourceDiagram() override = default;

  /// Called when an operation completes before updateSceneArcs().
  void updateSceneNodes(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) override;

  /// Called when an operation completes after updateSceneNodes().
  void updateSceneArcs(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) override;

  /// Return an item used to display metadata when a qtBaseNode is hovered.
  ///
  /// This item never has its configuration serialized.
  qtResourceDiagramSummary* summarizer() const { return m_summarizer; }

  /// Return the root node of the diagram (if it exists yet… it may not).
  qtBaseNode* root() const;

  /// Drag-and-drop support
  //@{

  /// Returns true if this generator wishes to handle the proposed drag-and-drop mime data.
  ///
  /// If your subclass returns true, then be prepared to receive 0 or more calls to moveDropPoint
  /// followed by either abortDrop or acceptDrop (depending on user input).
  bool acceptDropProposal(QDragEnterEvent* event) override
  {
    (void)event;
    return false;
  }

  /// Update any drop preview to the location in the provided \a event.
  ///
  /// This is only called after your acceptDropProposal method returns true.
  void moveDropPoint(QDragMoveEvent* event) override { (void)event; }

  /// Clean up any drop preview; the user has aborted the drag-and-drop.
  ///
  /// This may only be called after your acceptDropProposal method returns true.
  void abortDrop(QDragLeaveEvent* event) override { (void)event; }

  /// Return true if this generator accepts the drag-and-drop data at the finalized location.
  ///
  /// This may only be called after your acceptDropProposal method returns true.
  bool acceptDrop(QDropEvent* event) override
  {
    (void)event;
    return false;
  }

  //@}

  /// Save/load diagram state.
  //@{
  /// Add this generator's configuration data to the diagram's overall
  /// \a config.
  bool addConfiguration(nlohmann::json& config) const override;
  /// Fetch configuration data for this generator from the diagram's
  /// overall \a config.
  bool configure(const nlohmann::json& config) override;
  //@}

  /// Methods used by arc/node paint() methods to determine visual properties.
  //@{

  /// Set/get the opacity to use for short arcs (<= 3 hops).
  /// The default is 1.0;
  double shortArcOpacity() const { return m_shortArcOpacity; }
  bool setShortArcOpacity(double value);

  /// Set/get the amount to adjust opacity for extremely long arcs.
  /// The default is 0.75.
  ///
  /// Opacity for an arc with N hops is computed as O1 - O2 * (1 - exp(min(0, 3 - N))),
  /// where O1 is shortArcOpacity() and O2 is longArcOpacityAdjustment().
  double longArcOpacityAdjustment() const { return m_longArcOpacityAdjustment; }
  bool setLongArcOpacityAdjustment(double value);

  /// Set/get the control-polygon "tightness" adjustment factor, beta.
  ///
  /// Values near 1.0 produce arcs that conform closely to the tree.
  /// Values near 0.0 produce arcs that go nearly straight from node to node.
  /// The default is 0.9.
  ///
  /// This parameter is named β to match the Hierarchical Edge Bundles paper by Holten.
  double beta() const { return m_beta; }
  bool setBeta(double value);

  /// Set/get the spacing factor for placing nodes along the circumference of a circle.
  ///
  /// When the spacing factor is 1.0, the circle's radius is chosen such that nodes will
  /// be tightly packed around the circle (no space between nodes but no overlap).
  /// Values less than 1.0 will cause nodes to overlap. Values larger than 1.0 will
  /// leave space between nodes.
  ///
  /// The default is 1.125. Only positive values are accepted.
  double nodeSpacing() const { return m_nodeSpacing; }
  bool setNodeSpacing(double value);
  //@}

protected:
  /// Used by updateScene to cleans the layout of expunged nodes before removing the
  /// nodes from the scene.
  void removeFromLayout(const std::unordered_set<smtk::resource::PersistentObject*>& expunged);

  /// Create a layout of the nodes.
  ///
  /// If \a zoomToLayout is true, then qtDiagram::includeInView() will be called
  /// with the bounds of the diagram because 1 or more nodes/arcs have been modified.
  void generateLayout(bool zoomToLayout);

  bool acceptObject(const smtk::resource::PersistentObject* obj) const;

  template<bool HandleReparentedObjects>
  bool updateParentArc(
    smtk::resource::PersistentObject* object,
    ArcLegendEntries& registeredArcTypes);

  template<bool RemoveUnusedArcs>
  bool updateGraphArcs(
    smtk::resource::PersistentObject* object,
    ArcLegendEntries& registeredArcTypes);

  /// Hold a map of non-object nodes used to group objects together.
  /// (Note that object-based nodes may also group objects together,
  /// e.g., comments may be grouped underneath their subject node.)
  std::unordered_map<smtk::string::Token, qtBaseNode*> m_groupingNodes;

  /// The set of all leaf nodes and all arcs this diagram maintains.
  /// These are used to update a fixed layout for the diagram each time
  /// an operation runs.
  std::set<std::vector<qtBaseNode*>, VisualComparator> m_diagramNodes;
  std::unordered_set<qtBaseArc*> m_diagramArcs;

  /// An item that shows a summary of a node.
  qtResourceDiagramSummary* m_summarizer;

  /// Configuration information provided out-of-band by operations
  /// that produce application state in addition to changes to
  /// persistent objects.
  ///
  /// This particular information is currently (x,y)-locations,
  /// names, and node IDs of grouping nodes computed (or loaded off
  /// disk) by operations. It is provided via configure() during
  /// the operation and will eventually be used to populate
  /// m_groupingNodes by updateScene() (called by the operation's
  /// observers).
  nlohmann::json m_groupItemData;

  double m_shortArcOpacity{ 1.0 };
  double m_longArcOpacityAdjustment{ 0.75 };
  double m_beta{ 0.9 };
  double m_nodeSpacing{ 1.125 };
  std::unordered_set<smtk::string::Token> m_classExclusions;
  ObjectRules m_acceptsRules;
  ObjectRules m_rejectsRules;
  /// These ivars are used to store state between updateSceneNodes() and updateSceneArcs();
  /// they are not valid at all times.
  bool m_sceneModified{ false };
  bool m_addedToScene{ false };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtResourceDiagram_h
