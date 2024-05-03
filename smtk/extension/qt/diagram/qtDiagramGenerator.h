//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramGenerator_h
#define smtk_extension_qtDiagramGenerator_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/view/Information.h"

#include "smtk/operation/Operation.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeContainer.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QObject>

#include <unordered_set>

namespace smtk
{
namespace extension
{

class qtBaseArc;
class qtBaseNode;
class qtDiagram;
class qtDiagramLegendEntry;

/**\brief A base class that maintains a diagram in the face of operations which
  *       modify the state of the diagram's objective.
  */
class SMTKQTEXT_EXPORT qtDiagramGenerator : public QObject
{
  Q_OBJECT
public:
  smtkTypenameMacroBase(qtDiagramGenerator);
  smtkSuperclassMacro(QObject);

  /// Subclasses may use this to pass information about arc types registered with the legend.
  using ArcLegendEntries = std::unordered_map<smtk::string::Token, qtDiagramLegendEntry*>;

  /// Create a generator from configuration information.
  qtDiagramGenerator(
    const smtk::view::Information& info,
    const smtk::view::Configuration::Component& config,
    qtDiagram* parent);

  ~qtDiagramGenerator() override = default;

  /// Return the diagram this generator inserts content into.
  qtDiagram* diagram() const { return m_diagram; }

  /// Diagram update methods
  ///@{
  /// Called when an operation completes.
  ///
  /// Each diagram generator is invoked with the same values and a generator
  /// prior to this instance may already have created or expunged items in
  /// the scene corresponding to the \a created or \a expunged objects; be
  /// sure to check the diagram's node index before blindly adding/removing
  /// scene items.
  virtual void updateScene(
    std::unordered_set<smtk::resource::PersistentObject*>&,
    std::unordered_set<smtk::resource::PersistentObject*>&,
    std::unordered_set<smtk::resource::PersistentObject*>&,
    const smtk::operation::Operation&,
    const smtk::operation::Operation::Result&)
  {
  }

  virtual void updateSceneNodes(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result);
  virtual void updateSceneArcs(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) = 0;
  ///@}

  /// Called by subclasses to notify qtDiagramViewMode instances that the generator's
  /// items have been cleared.
  ///
  /// Modes which add scene items via a qtDiagramGenerator may need to re-insert items
  /// into the scene.
  void sceneCleared();

  /// Drag-and-drop support
  ///@{

  /// Returns true if this generator wishes to handle the proposed drag-and-drop mime data.
  ///
  /// If your subclass returns true, then be prepared to receive 0 or more calls to moveDropPoint
  /// followed by either abortDrop or acceptDrop (depending on user input).
  virtual bool acceptDropProposal(QDragEnterEvent* event)
  {
    (void)event;
    return false;
  }

  /// Update any drop preview to the location in the provided \a event.
  ///
  /// This is only called after your acceptDropProposal method returns true.
  virtual void moveDropPoint(QDragMoveEvent* event) { (void)event; }

  /// Clean up any drop preview; the user has aborted the drag-and-drop.
  ///
  /// This may only be called after your acceptDropProposal method returns true.
  virtual void abortDrop(QDragLeaveEvent* event) { (void)event; }

  /// Return true if this generator accepts the drag-and-drop data at the finalized location.
  ///
  /// This may only be called after your acceptDropProposal method returns true.
  virtual bool acceptDrop(QDropEvent* event)
  {
    (void)event;
    return false;
  }

  ///@}

  /// Save/load diagram state.
  ///@{
  /// Add this generator's configuration data to the diagram's overall
  /// \a config.
  virtual bool addConfiguration(nlohmann::json& config) const
  {
    (void)config;
    return true;
  }
  /// Fetch configuration data for this generator from the diagram's
  /// overall \a config.
  virtual bool configure(const nlohmann::json& config)
  {
    (void)config;
    return true;
  }
  ///@}

public Q_SLOTS:
  /// Identify arcs that need updating when a node is moved.
  ///
  /// The node is identified by examining the sender of the signal,
  /// not by an argument. Then, updateArcsOfNodeRecursive() is called with that node.
  void updateArcsOfSendingNodeRecursive();

  /// Identify arcs that need updating when a node (not necessarily a leaf node) is moved.
  void updateArcsOfNodeRecursive(qtBaseNode* node);

protected:
  /// Used by updateArcsOfNodeRecursive to find arcs attached to node *and* all its children.
  void addArcsOfNodeRecursive(qtBaseNode* node, std::unordered_set<qtBaseArc*>& arcs);

  /// The parent diagram.
  qtDiagram* m_diagram{ nullptr };
};

} // namespace extension
} // namespace smtk
#endif // smtk_extension_qtDiagramGenerator_h
