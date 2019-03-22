//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_AvailableOperations_h
#define smtk_view_AvailableOperations_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/Observers.h"

#include "smtk/workflow/OperationFilterSort.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/MetadataObserver.h"

#include "smtk/resource/Component.h"

#include "smtk/view/SelectionObserver.h"

#include <set>
#include <vector>

namespace smtk
{
namespace view
{

/// Maintain a list of operations that may be applied to a selection.
class SMTKCORE_EXPORT AvailableOperations : smtkEnableSharedPtr(AvailableOperations)
{
public:
  smtkTypeMacroBase(smtk::view::AvailableOperations);
  smtkCreateMacro(AvailableOperations);
  using Index = smtk::operation::Operation::Index;
  using Observer = std::function<void(AvailableOperations::Ptr)>;
  using Observers = smtk::common::Observers<Observer>;
  // TODO: this should be OperationManagerPtr to maintain naming consistency.
  using OperationManager = smtk::operation::ManagerPtr;
  using OperationIndexSet = std::set<Index>;
  using OperationIndexArray = std::vector<Index>;
  // TODO: this should be OperationFilterSortPtr to maintain naming consistency.
  using OperationFilterSort = smtk::workflow::OperationFilterSortPtr;
  using Data = smtk::workflow::OperationFilterSort::Data;
  virtual ~AvailableOperations();

  /// The selection for which we should list applicable operations.
  SelectionPtr selection() const { return m_selection; }
  /// Set the selection for which we should list applicable operations.
  void setSelection(SelectionPtr seln);

  /**\brief Return whether or not the selection should be considered when listing operations.
    *
    * The default is true.
    * If false, then all operations that the workflow filter chooses to expose
    * will be listed as available. Otherwise, only the subset which may be directly
    * applied to the current selection are listed as available.
    */
  bool useSelection() const { return m_useSelection; }
  /// Set whether the selection should be considered when listing operations.
  void setUseSelection(bool useSeln);

  /// The operator manager from which we draw candidate operations.
  smtk::operation::ManagerPtr operationManager() const { return m_operationManager; }
  /// Set the operator manager from which we draw candidate operations.
  void setOperationManager(smtk::operation::ManagerPtr mgr);

  /// Return an object maintained by the workflow that filters and sorts candidate operations.
  OperationFilterSort workflowFilter() const { return m_workflowFilter; }
  void setWorkflowFilter(OperationFilterSort wf);

  /// Return the observers associated with this phrase model.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Called when operations are registered/unregistered from the operation manager.
  void operationMetadataChanged(const smtk::operation::Metadata& operMeta, bool adding);
  /// Called when the selection is modified; computes a new list of operations.
  void selectionModified(const std::string& src, SelectionPtr seln);
  /// Called when the workflow filter settings change.
  void workflowFilterModified();

  /// Return the intermediate working set of available but unfiltered operations.
  OperationIndexSet& workingSet() { return m_workingSet; }
  OperationIndexSet workingSet() const { return m_workingSet; }

  /// Return the list of operations.
  OperationIndexArray& availableOperations() { return m_available; }
  OperationIndexArray availableOperations() const { return m_available; }

  /// Return data describing how to present an operation (a convenience from the OperationFilterSort).
  const Data* operationData(const Index& opIdx) const;

  /**\brief Core functionality of the class as a static method to find operations that apply to a selection.
    *
    * Note that it is valid to pass a null pointer to \a selectionIn.
    * In this case, all operations in the manager are returned.
    */
  static void workingSet(smtk::operation::ManagerPtr operationsIn,
    smtk::view::SelectionPtr selectionIn, int selectionMaskIn, bool exactSelectionIn,
    OperationIndexSet& workingSet);

  /// Core functionality of the class as a static method to filter operations for presentation:
  static void availableOperations(
    const OperationIndexSet& workingSetIn, OperationFilterSort filterIn, OperationIndexArray& out);

protected:
  AvailableOperations();

  /// When the selection or operation manager signal changes, update both the working set and final output.
  void computeFromSelection();
  /// When the workflow signals a change, use the existing working set to compute a new final output.
  void computeFromWorkingSet();

  OperationManager m_operationManager;
  smtk::operation::MetadataObservers::Key m_operationManagerObserverId;
  SelectionPtr m_selection;
  SelectionObservers::Key m_selectionObserverId;
  int m_selectionMask;
  bool m_selectionExact;
  bool m_useSelection;
  OperationFilterSort m_workflowFilter;
  smtk::workflow::OperationFilterSort::Observers::Key m_workflowFilterObserverId;
  OperationIndexSet m_workingSet;
  OperationIndexArray m_available;
  Observers m_observers;
};
}
}

#endif
