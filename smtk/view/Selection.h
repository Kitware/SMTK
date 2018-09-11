//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_Selection_h
#define smtk_view_Selection_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/resource/Component.h"

#include <functional>
#include <map>
#include <set>

namespace smtk
{
namespace view
{

/// Descriptors for how lists passed to the selection should modify the selection-map.
enum class SelectionAction
{
  /**\brief Replace all the current selection and filter it.
    *
    * Example use case: normal selection from rendering window.
    */
  FILTERED_REPLACE = 0,
  /**\brief Replace all the current selection and do not filter it.
    *
    * Example use case: selection from model tree and attribute panel.
    */
  UNFILTERED_REPLACE = 1,
  /**\brief Filter the input selection and add it to the current selection.
    *
    * Example use case: addition from rendering window.
    */
  FILTERED_ADD = 2,
  /**\brief Do not filter the input selection, just add it to the current selection.
    *
    * Example use case: addition from operator dialog where a the user is
    * presented with a pre-filtered list.
    */
  UNFILTERED_ADD = 3,
  /**\brief Filter the input selection, then subtract it from the current selection.
    *
    * Example use case: substraction from rendering window.
    */
  FILTERED_SUBTRACT = 4,
  /**\brief Do not filter the input selection, just subtract it from the current selection.
    *
    * Example use case: subtraction from operator dialog where a pre-filtered list is provided.
    */
  UNFILTERED_SUBTRACT = 5,
  /**\brief Use the default SelectionAction provided by the Selection.
    *
    * Use it when SelectionAction should be decided by previous user input
    * that resulted in a mode being set on the Selection.
    *
    * An example is when a user presses a modifier key while in a selection mode
    * to switch between addition, subtraction, or replacement. In this case,
    * the key presses will change the default action on the selection
    * while the mouse clicks that identify entities signal their intent
    * to use the updated default with this enum.
    */
  DEFAULT = 6
};

/**\brief A class to manage linked selections in a context.
  *
  * This class allows multiple UI elements to share a selection
  * by providing methods to modify and filter the selection as
  * well as receive signals when the selection is updated by others.
  *
  * The major use case is to support a single, application-wide
  * selection but it is possible to create multiple instances
  * of this class if your application wishes to manage different
  * selections depending upon some context.
  * The instance() method supports application-wide
  * selections by returning the first selection instance which has
  * been created (or creates a new instance if its weak pointer is
  * null).
  *
  * ## The Nature of Selections
  *
  * A selection is a map from ResourceComponents(UUIDs) to
  * integers that indicate the _type_ of selection that the
  * component is participating in.
  * Integers are application-specific and managed in this
  * class by registering them.
  * Applications are free to interpret the integer as a
  * bitmask, so that the same component may be in multiple
  * selections at a time; or as a unique values that indicate
  * the "level of selection" for each component.
  * An example of the latter would be an application that
  * registers 1 to indicate a transient highlight to preview
  * mouseovers and 2 to indicate permanent selection.
  *
  * When a component is assigned the special integer 0,
  * the component is considered "unselected" and is removed
  * from the map.
  *
  * ## Lifecycle
  *
  * First, your application should create a selection instance for
  * each separate context in which a separate selection is allowed.
  * If your application will be making changes to resources by
  * invoking operators, you should register the operation manager with
  * the selection so that as resource components are created and
  * destroyed the selection can be updated.
  *
  * After creating a selection, each UI element of your
  * application that will (a) share the selection and (b) modify
  * the selection should register itself as a source of selections
  * by providing a unique string name to registerSelectionSource().
  *
  * Similarly, each UI element of your application that will (a) share
  * the selection and (b) present the selection or otherwise need to
  * be informed of changes to the selection should register as a
  * listener by providing a callback to observe().
  *
  * ## Selection Events
  *
  * When the selection changes, listener functions subscribed to
  * updates will be called with the name of the UI elements that
  * caused the event (or "selection" if a change inside the
  * selection itself caused the event). A pointer to the selection
  * is also provided so you can query the selection
  * inside the listener function. You should **not** modify
  * the selection inside a listener as that can cause infinite
  * recursion.
  *
  * The listener may be called under a variety of circumstances:
  *
  * + upon initial registration with the selection (if
  *   \a immediatelyNotify is passed as true),
  * + upon modification by a selection source
  * + upon a change to the selection's filter that
  *   results in a change to the selection.
  *
  * While listeners are not usually informed when attempted
  * changes to the selection have no effect, it is possible
  * to get called when the selection is entirely replaced with
  * an identical selection.
  */
class SMTKCORE_EXPORT Selection : smtkEnableSharedPtr(Selection)
{
public:
  using Component = smtk::resource::Component;
  using Object = smtk::resource::PersistentObject;

  smtkTypeMacroBase(Selection);
  smtkCreateMacro(Selection);

  static Ptr instance();

  /// This is the type of function used to notify observers when the selection is modified.
  using Observer = std::function<void(const std::string&, Selection::Ptr)>;

  /// This is the underlying storage type that holds selections.
  using SelectionMap = std::map<Object::Ptr, int>;

  /**\brief Selection filters take functions of this form.
    *
    * Given a component and its selection "value", return true if
    * the component should be included in the action (replacement,
    * addition to, or removal from the selection). Otherwise return false.
    *
    * The filter may also insert components into the map (the 3rd argument)
    * which will unconditionally be included in the action.
    * This is intended to handle use cases where related components
    * (e.g., edges, faces, vertices) may be geometrically picked by a user
    * but the desired selection is on components not directly rendered
    * (e.g., volumes, models). The filter can return false (indicating that
    * the edge, face, or vertex should not be considered) but add the related
    * entities (the volume or model) to the map for action.
    */
  using SelectionFilter = std::function<bool(Object::Ptr, int, SelectionMap&)>;

  virtual ~Selection();

  /**\brief Selection sources.
    *
    * Selections are modified by UI elements, each of which is considered a _source_
    * of the selection. The selection provides a way for these UI elements
    * to register in order to verify that their name is unique (within the selection).
    */
  //@{
  /**\brief Register a selection source.
    *
    * If false is returned, the \a name is already in use;
    * try again with a different \a name.
    */
  bool registerSelectionSource(std::string name) { return m_selectionSources.insert(name).second; }

  /// Unregister a selection source; if false is returned, the source was not recognized.
  bool unregisterSelectionSource(std::string name) { return (m_selectionSources.erase(name) > 0); }

  /// Populate a set with the names of all registered selection sources.
  const std::set<std::string>& getSelectionSources() const { return m_selectionSources; }

  /// Populate a set with the names of all registered selection sources.
  void getSelectionSources(std::set<std::string>& selectionSources) const
  {
    selectionSources = m_selectionSources;
  }
  //@}

  /**\brief Selection values.
    *
    * Selected components each take on a non-zero integer value that is application-defined.
    *
    * The methods in this section are used to register particular values or bits that the
    * application wishes to use. Each value is tied to a string name.
    */
  //@{
  /// Register a selection value. If the value was already registered, returns false.
  bool registerSelectionValue(
    const std::string& valueLabel, int value, bool valueMustBeUnique = true);
  /// Unregister a selection value. If the value was not registered, returns false.
  bool unregisterSelectionValue(const std::string& valueLabel)
  {
    return m_selectionValueLabels.erase(valueLabel) > 0;
  }
  /// Unregister a selection value. If the value was not registered, returns false.
  bool unregisterSelectionValue(int value);
  /// Return the map of selection values.
  const std::map<std::string, int>& selectionValueLabels() const { return m_selectionValueLabels; }
  /// Return the selection value for the given \a label, or 0 if there is no such label registered.
  int selectionValueFromLabel(const std::string& label) const;
  /// Return the selection value for the given \a label, registering a new value if needed.
  int findOrCreateLabeledValue(const std::string& label);
  //@}

  /**\brief Modify the current selection.
    *
    * Returns true if the arguments result in a change to the selection.
    *
    * \a action specifies indicates whether the list of
    * \a components should replace the selection, be added to it, or be removed from it.
    * \a value indicates the "level" at which the \a components should be selected
    * (except when the \a action is a subtraction operation, in which case \a value is ignored).
    * \a source indicates which UI element is responsible for the selection modification.
    */
  template <typename T>
  bool modifySelection(const T& components, const std::string& source, int value,
    SelectionAction action = SelectionAction::DEFAULT);

  /**\brief Default selection action.
    *
    * Some applications need to separate the choice of which SelectionAction
    * to use from the choice of components in the selection.
    * The selection keeps a default SelectionAction value as state so that
    * when modifySelection is called with the \a action set to SelectionAction::DEFAULT, the
    * selection's state is used in its stead.
    *
    * The methods in this section provide a way to set and query the action
    * used when SelectionAction::DEFAULT is passed to modifySelection.
    * Obviously, SelectionAction::DEFAULT is not allowed.
    * You are strongly encouraged only to allow FILTERED and **not** UNFILTERED actions
    * as the default.
    *
    * The initial value for the default action is FILTERED_REPLACE.
    */
  //@{
  /// Set the selection modifier. Returns true if \a action was valid; false otherwise.
  bool setDefaultAction(const SelectionAction& action);

  /// Return the current method used to modify selections when
  SelectionAction defaultAction() const { return m_defaultAction; }

  void setDefaultActionToReplace() { m_defaultAction = SelectionAction::FILTERED_REPLACE; }

  void setDefaultActionToAddition() { m_defaultAction = SelectionAction::FILTERED_ADD; }

  void setDefaultActionToSubtraction() { m_defaultAction = SelectionAction::FILTERED_SUBTRACT; }
  //@}

  /** \brief Querying the current selection.
    *
    * You can obtain the current selection in bulk or components selected
    * with a particular value.
    * If you are using selection values as independent bits in a it vector,
    * pass \a exactMatch = false.
    */
  //@{
  /// Visit every selected component with the given functor.
  void visitSelection(std::function<void(Object::Ptr, int)> visitor)
  {
    for (auto entry : m_selection)
    {
      visitor(entry.first, entry.second);
    }
  }
  void visitSelection(std::function<void(Component::Ptr, int)> visitor)
  {
    for (auto entry : m_selection)
    {
      Component::Ptr val = std::dynamic_pointer_cast<Component>(entry.first);
      if (val)
      {
        visitor(val, entry.second);
      }
    }
  }
  /// Return the current selection as a map from components to integer selection values.
  SelectionMap& currentSelection(SelectionMap& selection) const;
  const SelectionMap& currentSelection() const { return m_selection; }
  /// Return the subset of selected elements that match the given selection value.
  template <typename T>
  T& currentSelectionByValue(T& selection, int value, bool exactMatch = true);
  template <typename T>
  T& currentSelectionByValue(T& selection, const std::string& valueLabel, bool exactMatch = true);

  template <typename T>
  T currentSelectionByValueAs(int value, bool exactMatch = true)
  {
    T result;
    return this->currentSelectionByValue(result, value, exactMatch);
  }
  template <typename T>
  T currentSelectionByValueAs(const std::string& valueLabel, bool exactMatch = true)
  {
    T result;
    return this->currentSelectionByValue(result, valueLabel, exactMatch);
  }
  //@}

  /** \brief Selection Events
    *
    */
  //@{
  /// Call \a fn whenever the selection is modified. Returns a handle you can pass to unobserve().
  int observe(Observer fn, bool immediatelyNotify = false);

  /// Stop listening to selection events.
  bool unobserve(int handle) { return m_listeners.erase(handle) > 0; }
  //@}

  /** \brief Selection filtering.
    *
    */
  //@{
  /// Set the filter to apply to each resource component.
  void setFilter(const SelectionFilter& fn, bool refilterSelection = true);
  //@}

protected:
  Selection();

  bool performAction(smtk::resource::PersistentObjectPtr comp, int value, SelectionAction action,
    SelectionMap& suggested);
  void notifyListeners(const std::string& source);
  bool refilter(const std::string& source);

  SelectionAction m_defaultAction;
  //smtk::model::BitFlags m_modelEntityMask;
  bool m_meshSetMask;
  std::set<std::string> m_selectionSources;
  std::map<std::string, int> m_selectionValueLabels;
  SelectionMap m_selection;
  std::map<int, Observer> m_listeners;
  SelectionFilter m_filter;

private:
  Selection(const Selection&) = delete;
  void operator=(const Selection&) = delete;
};

template <typename T>
T& Selection::currentSelectionByValue(T& selection, int value, bool exactMatch)
{
  if (exactMatch)
  {
    for (auto entry : m_selection)
    {
      if ((entry.second & value) == value)
      {
        auto entryT = std::dynamic_pointer_cast<typename T::value_type::element_type>(entry.first);
        if (entryT)
        {
          selection.insert(selection.end(), entryT);
        }
      }
    }
  }
  else
  {
    for (auto entry : m_selection)
    {
      if (entry.second & value)
      {
        auto entryT = std::dynamic_pointer_cast<typename T::value_type::element_type>(entry.first);
        if (entryT)
        {
          selection.insert(selection.end(), entryT);
        }
      }
    }
  }
  return selection;
}

template <typename T>
T& Selection::currentSelectionByValue(T& selection, const std::string& valueLabel, bool exactMatch)
{
  int val = this->selectionValueFromLabel(valueLabel);
  return this->currentSelectionByValue(selection, val, exactMatch);
}

template <typename T>
bool Selection::modifySelection(
  const T& components, const std::string& source, int value, SelectionAction action)
{
  bool modified = false;
  SelectionMap suggestions;
  if (action == SelectionAction::DEFAULT)
  {
    action = this->defaultAction();
  }
  if (action == SelectionAction::FILTERED_REPLACE || action == SelectionAction::UNFILTERED_REPLACE)
  {
    modified = !m_selection.empty();
    m_selection.clear();
  }
  for (auto component : components)
  {
    modified |= this->performAction(component, value, action, suggestions);
  }
  if (modified)
  {
    this->notifyListeners(source);
  }
  return modified;
}

} // namespace view
} // namespace smtk

#endif
