//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/SelectionManager.h"

#include "smtk/resource/Component.h"

#define SMTK_SELECTION_MANAGER_SOURCE "selection manager"

namespace smtk
{
namespace resource
{

static bool defaultFilter(
  ComponentPtr comp, int selectionValue, SelectionManager::SelectionMap& suggestions)
{
  (void)comp;
  (void)selectionValue;
  (void)suggestions;
  return true;
}

SelectionManager::SelectionManager()
  : m_defaultAction(SelectionAction::FILTERED_REPLACE)
  , m_filter(defaultFilter)
{
}

SelectionManager::~SelectionManager()
{
  // We would like to notify remaining listeners if the selection
  // is non-empty here, but cannot since shared_from_this() doesn't
  // work inside the destructor.
}

bool SelectionManager::registerSelectionValue(
  const std::string& valueLabel, int value, bool valueMustBeUnique)
{
  if (value == 0)
  { // 0 is not allowed; it corresponds to being unselected.
    return false;
  }

  if (valueMustBeUnique)
  {
    for (auto entry : m_selectionValueLabels)
    {
      if (entry.second == value)
      {
        return false;
      }
    }
  }

  return this->m_selectionValueLabels.insert(std::make_pair(valueLabel, value)).second;
}

bool SelectionManager::unregisterSelectionValue(int value)
{
  bool didErase = false;
  for (auto it = this->m_selectionValueLabels.begin(); it != this->m_selectionValueLabels.end();)
  {
    auto tmp = it;
    ++it; // Keep from invalidating our current iterator.
    if (tmp->second == value)
    {
      this->m_selectionValueLabels.erase(tmp);
      didErase = true;
    }
  }
  return didErase;
}

int SelectionManager::selectionValueFromLabel(const std::string& label) const
{
  auto it = this->m_selectionValueLabels.find(label);
  if (it == this->m_selectionValueLabels.end())
  {
    return 0;
  }
  return it->second;
}

int SelectionManager::findOrCreateLabeledValue(const std::string& label)
{
  auto it = this->m_selectionValueLabels.find(label);
  if (it == this->m_selectionValueLabels.end())
  {
    // Need to find a unique value.
    int handle = static_cast<int>(this->m_selectionValueLabels.size()) + 1;
    while (!this->registerSelectionValue(label, handle))
    {
      ++handle;
    }
    return handle;
  }
  return it->second;
}

bool SelectionManager::setDefaultAction(const SelectionAction& action)
{
  switch (action)
  {
    case SelectionAction::FILTERED_REPLACE:
    case SelectionAction::FILTERED_ADD:
    case SelectionAction::FILTERED_SUBTRACT:
    case SelectionAction::UNFILTERED_REPLACE:
    case SelectionAction::UNFILTERED_ADD:
    case SelectionAction::UNFILTERED_SUBTRACT:
      this->m_defaultAction = action;
      return true;
    default:
      break;
  }
  return false;
}

SelectionManager::SelectionMap& SelectionManager::currentSelection(SelectionMap& selection) const
{
  selection = m_selection;
  return selection;
}

int SelectionManager::listenToSelectionEvents(Listener fn, bool immediatelyNotify)
{
  if (!fn)
  {
    return -1;
  }

  int handle = this->m_listeners.empty() ? 0 : this->m_listeners.rbegin()->first + 1;
  this->m_listeners[handle] = fn;
  if (immediatelyNotify)
  {
    fn(SMTK_SELECTION_MANAGER_SOURCE, shared_from_this());
  }
  return handle;
}

void SelectionManager::setFilter(const SelectionFilter& fn, bool refilter)
{
  if (!fn)
  {
    // We require a valid filter always. Use our default, which accepts everything:
    this->m_filter = defaultFilter;
    // No need to re-run filter, since we know it will accept everything in the current selection.
    return;
  }

  // We cannot detect whether fn == m_filter, because "==" has been deleted from std::function.
  // If we could, we would return early here.

  // Set the filter and run it on the existing selection if instructed.
  this->m_filter = fn;
  if (refilter)
  {
    this->refilter(SMTK_SELECTION_MANAGER_SOURCE);
  }
}

/// Perform the action (IGNORING m_defaultAction!!!), returning true if it had an effect
bool SelectionManager::performAction(
  smtk::resource::ComponentPtr comp, int value, SelectionAction action, SelectionMap& suggestions)
{
  bool modified = false;
  // Filter out irrelevant components:
  switch (action)
  {
    case SelectionAction::FILTERED_REPLACE:
    case SelectionAction::FILTERED_ADD:
    case SelectionAction::FILTERED_SUBTRACT:
      if (!m_filter(comp, value, suggestions))
      {
        // Add the suggested entries if any.
        for (auto suggestion : suggestions)
        {
          auto it = m_selection.find(suggestion.first);
          if (it == m_selection.end())
          {
            if (suggestion.second != 0)
            {
              modified = true;
              m_selection.insert(suggestion);
            }
          }
          else if (it->second != suggestion.second)
          {
            modified = true;
            if (suggestion.second == 0)
            {
              m_selection.erase(it);
            }
            else
            {
              it->second = suggestion.second;
            }
          }
        }
        suggestions.clear();
        return modified;
      }
    default:
      break;
  };

  // Replace (which is equivalent to add inside performAction), add, or subtract:
  auto it = this->m_selection.find(comp);
  switch (action)
  {
    case SelectionAction::FILTERED_REPLACE:
    case SelectionAction::UNFILTERED_REPLACE:
    case SelectionAction::FILTERED_ADD:
    case SelectionAction::UNFILTERED_ADD:
      if (it == this->m_selection.end())
      {
        modified = true;
        this->m_selection[comp] = value;
      }
      else if (it->second != value)
      {
        modified = true;
        it->second = value;
      }
      // Now add all the suggested entries and clear.
      for (auto suggestion : suggestions)
      {
        it = m_selection.find(suggestion.first);
        if (it == m_selection.end())
        {
          if (suggestion.second != 0)
          {
            modified = true;
            m_selection.insert(suggestion);
          }
        }
        else if (it->second != suggestion.second)
        {
          modified = true;
          if (suggestion.second == 0)
          {
            m_selection.erase(it);
          }
          else
          {
            it->second = suggestion.second;
          }
        }
      }
      suggestions.clear();
      break;
    case SelectionAction::FILTERED_SUBTRACT:
    case SelectionAction::UNFILTERED_SUBTRACT:
      if (it != m_selection.end())
      {
        modified = true;
        this->m_selection.erase(it);
      }
      // Now deal with suggestions... should we really allow additions
      // during a subtract? Not going to for now, but I guess it is
      // possible someone might want to make a substitution.
      for (auto suggestion : suggestions)
      {
        it = m_selection.find(suggestion.first);
        if (it != m_selection.end() && suggestion.second == 0)
        {
          modified = true;
          m_selection.erase(it);
        }
      }
      suggestions.clear();
      break;
    default:
      break;
  }
  return modified;
}

void SelectionManager::notifyListeners(const std::string& source)
{
  Ptr self = this->shared_from_this();
  for (auto ll : m_listeners)
  {
    ll.second(source, self);
  }
}

bool SelectionManager::refilter(const std::string& source)
{
  SelectionMap suggestions;
  bool modified = false;
  for (auto it = m_selection.begin(); it != m_selection.end();)
  {
    if (!this->m_filter(it->first, it->second, suggestions))
    { // Remove the current item from the selection, being careful not to invalidate the iterator:
      auto tmp = it;
      modified = true;
      ++it;
      m_selection.erase(tmp);
    }
    else
    { // We still like the current item; keep it.
      ++it;
    }
  }
  // Now handle suggestions
  for (auto suggestion : suggestions)
  {
    auto it = m_selection.find(suggestion.first);
    if (it == m_selection.end())
    {
      if (suggestion.second != 0)
      {
        modified = true;
        m_selection.insert(suggestion);
      }
    }
    else if (it->second != suggestion.second)
    {
      modified = true;
      if (suggestion.second == 0)
      {
        m_selection.erase(it);
      }
      else
      {
        it->second = suggestion.second;
      }
    }
  }
  suggestions.clear();
  if (modified)
  {
    this->notifyListeners(source);
  }
  return modified;
}
}
}
