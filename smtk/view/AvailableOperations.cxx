//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/AvailableOperations.h"

#include "smtk/view/Selection.h"

#include "smtk/workflow/OperationFilterSort.h"

#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/InternalGroup.h"

#include "smtk/attribute/ReferenceItemDefinition.h"

#define DEBUG_AVAILABLE_OPERATIONS 0

using namespace smtk::view;

AvailableOperations::AvailableOperations()
  : m_workflowFilter(nullptr)
{
// For debugging:
#if !defined(NDEBUG) && DEBUG_AVAILABLE_OPERATIONS
  static auto observerKey = this->observers().insert(
    [](AvailableOperations::Ptr self) {
      std::cout << "Available ops changed:\n";
      auto& mdByIdx = self->operationManager()->metadata().get<smtk::operation::IndexTag>();
      auto& avail = self->availableOperations();
      for (auto idx : avail)
      {
        auto mit = mdByIdx.find(idx);
        if (mit != mdByIdx.end())
        {
          std::cout << "  " << mit->typeName() << "\n";
        }
      }
    },
    0,     // assign a neutral priority
    false, // do not immediately invoke.
    "AvailableOperations: Print available operations.");
#endif
}

AvailableOperations::~AvailableOperations()
{
  if (m_operationManager)
  {
    m_operationManager->metadataObservers().erase(m_operationManagerObserverId);
  }
  if (m_selection)
  {
    m_selection->observers().erase(m_selectionObserverId);
  }
}

void AvailableOperations::setSelection(SelectionPtr seln)
{
  if (seln == m_selection)
  {
    return;
  }
  if (m_selection)
  {
    m_selection->observers().erase(m_selectionObserverId);
  }
  m_selection = seln;
  if (m_selection)
  {
    m_selectionObserverId = m_selection->observers().insert(
      [this](const std::string& src, smtk::view::Selection::Ptr seln) {
        this->selectionModified(src, seln);
      },
      0,    // assign a neutral priority
      true, // immediately notify
      "Update available operations list to reflect selection change");
  }
}

void AvailableOperations::setUseSelection(bool useSeln)
{
  if (m_useSelection == useSeln)
  {
    return;
  }

  m_useSelection = useSeln;

  // We only need to recompute when we have a selection.
  // Otherwise, m_useSelection has no effect.
  if (m_selection)
  {
    this->computeFromSelection();
  }
}

void AvailableOperations::setOperationManager(smtk::operation::ManagerPtr mgr)
{
  if (mgr == m_operationManager)
  {
    return;
  }

  if (m_operationManager)
  {
    m_operationManager->metadataObservers().erase(m_operationManagerObserverId);
  }
  m_operationManager = mgr;
  if (m_operationManager)
  {
    m_operationManagerObserverId = m_operationManager->metadataObservers().insert(
      [this](const smtk::operation::Metadata& operMeta, bool adding) {
        this->operationMetadataChanged(operMeta, adding);
      },
      "Update available operations to reflect change in operation metadata");
  }
  else
  {
    m_operationManagerObserverId = smtk::operation::MetadataObservers::Key();
  }
}

void AvailableOperations::setWorkflowFilter(OperationFilterSort wf)
{
  if (wf == m_workflowFilter)
  {
    return;
  }

  if (m_workflowFilter)
  {
    m_workflowFilter->observers().erase(m_workflowFilterObserverId);
  }
  m_workflowFilter = wf;
  if (m_workflowFilter)
  {
    m_workflowFilterObserverId = m_workflowFilter->observers().insert(
      [this]() { this->workflowFilterModified(); },
      "Update available operations to reflect a change in the workflow filter");
  }
  else
  {
    m_workflowFilterObserverId = smtk::workflow::OperationFilterSort::Observers::Key();
  }
}

void AvailableOperations::operationMetadataChanged(
  const smtk::operation::Metadata& operMeta,
  bool adding)
{
  (void)operMeta;
  (void)adding;
  // std::cout << "Operation \"" << operMeta.typeName() << "\" idx " << operMeta.index() << " add " << (adding ? "Y" : "N") <<  "\n";
  this->computeFromSelection();
}

void AvailableOperations::selectionModified(const std::string& src, SelectionPtr seln)
{
  (void)src;  // We are never the source, so we don't need to terminate early to avoid recursion.
  (void)seln; // We already have the selection, so ignore this, too.
  this->computeFromSelection();
}

void AvailableOperations::workflowFilterModified()
{
  this->computeFromWorkingSet();
}

const AvailableOperations::Data* AvailableOperations::operationData(const Index& opIdx) const
{
  if (!m_workflowFilter)
  {
    return nullptr;
  }

  const auto& filterList = m_workflowFilter->filterList();
  auto it = filterList.find(opIdx);
  if (it == filterList.end())
  {
    return nullptr;
  }
  return &it->second;
}

void AvailableOperations::workingSet(
  smtk::operation::ManagerPtr operationsIn,
  smtk::view::SelectionPtr selectionIn,
  int selectionMaskIn,
  bool exactSelectionIn,
  OperationIndexSet& workingSetOut)
{
  smtk::operation::InternalGroup internalOperations(operationsIn);

  workingSetOut.clear();
  if (selectionIn)
  {
    std::map<smtk::operation::Operation::Index, int> counts;
    const auto& selnMap = selectionIn->currentSelection();
    // Narrow the selection map down to the actual selected
    // set based on how the application wants us to use the selection:
    std::set<smtk::resource::PersistentObjectPtr> actual;
    for (const auto& entry : selnMap)
    {
      if (
        (exactSelectionIn && ((entry.second & selectionMaskIn) == selectionMaskIn)) ||
        (!exactSelectionIn && (entry.second & selectionMaskIn)))
      {
        actual.insert(entry.first);
      }
    }

    for (const auto& md : operationsIn->metadata())
    {
      auto primaryAssociation = md.primaryAssociation();
      std::size_t numRequired =
        primaryAssociation ? primaryAssociation->numberOfRequiredValues() : 0;
      std::size_t maxAllowed = primaryAssociation ? primaryAssociation->maxNumberOfValues() : 0;
      std::size_t numSel = static_cast<std::size_t>(actual.size());
      bool extensible = primaryAssociation ? primaryAssociation->isExtensible() : false;
      bool optional = primaryAssociation ? primaryAssociation->isOptional() : false;

      // Maybe we can terminate early:
      if (numSel == 0 && (!primaryAssociation || optional || numRequired == 0))
      {
        // The actual selection is empty and the operation does not want
        // or need associations, mark it as available.
        if (!internalOperations.contains(md.index()))
        {
          workingSetOut.insert(md.index());
        }
        continue;
      }
      else if (numSel == 0)
      {
        // The actual selection is empty but we have an association
        // rule that requires entries, so the operation is unavailable.
        continue;
      }
      else if (!primaryAssociation)
      {
        // The actual selection is non-empty and we don't want
        // want entries... the operation is unavailable.
        continue;
      }

      // Now we know there's an association and we have a non-empty
      // selection so there's a chance the operation should be available.
      if (maxAllowed > 0 && numSel > maxAllowed)
      {
        // Too many items selected.
        continue;
      }
      else if (numSel < numRequired)
      {
        // Too few items selected.
        continue;
      }
      else if (numSel > numRequired && !extensible)
      {
        continue;
      }

      // All the easy checks are done; see if the number and type
      // of items in the actual selection exactly match the requirements.
      bool match = true;
      for (const auto& item : actual)
      {
        if (!primaryAssociation->isValueValid(item))
        {
          match = false;
          break; // Nope, not a match.
        }
      }
      if (match)
      {
        if (
          (!extensible && numSel == numRequired) ||
          (extensible && numSel >= numRequired && (maxAllowed == 0 || numSel <= maxAllowed)))
        {
          // Do not present operations marked as internal
          if (!internalOperations.contains(md.index()))
          {
            workingSetOut.insert(md.index());
          }
        }
      }
    }
  }
  else
  {
    for (const auto& md : operationsIn->metadata())
    {
      // Do not present operations marked as internal
      if (!internalOperations.contains(md.index()))
      {
        workingSetOut.insert(md.index());
      }
    }
  }
}

void AvailableOperations::availableOperations(
  const OperationIndexSet& workingSetIn,
  smtk::workflow::OperationFilterSortPtr filterIn,
  OperationIndexArray& out)
{
  if (filterIn)
  {
    filterIn->apply(workingSetIn, out);
  }
  else
  {
    out.clear();
    out.insert(out.end(), workingSetIn.begin(), workingSetIn.end());
  }
}

void AvailableOperations::computeFromSelection()
{
  if (!m_operationManager || m_operationManager->metadata().empty())
  {
    return;
  }
  OperationIndexSet workingSet;
  AvailableOperations::workingSet(
    m_operationManager,
    m_useSelection ? m_selection : nullptr,
    m_selectionMask,
    m_selectionExact,
    workingSet);
  if (m_workingSet == workingSet)
  {
    // The selection changed, but the set of available operators didn't.
    return;
  }
  // Now update the output array based on the working set:
  m_workingSet = workingSet;
  this->computeFromWorkingSet();
}

void AvailableOperations::computeFromWorkingSet()
{
  availableOperations(m_workingSet, m_workflowFilter, m_available);
  this->observers()(shared_from_this());
}
