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
#include "smtk/view/SelectionAction.h"
#include "smtk/view/SelectionObserver.h"

#include <functional>
#include <map>
#include <set>

namespace smtk
{
namespace view
{

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
  * A selection is a map from PersistentObjects to
  * integers that indicate the _type_ of selection that the
  * object is participating in.
  * Integers are application-specific and managed in this
  * class by registering them.
  * Applications are free to interpret the integer as a
  * bitmask, so that the same object may be in multiple
  * selections at a time; or as a unique values that indicate
  * the "level of selection" for each object.
  * An example of the latter would be an application that
  * registers 1 to indicate a transient highlight to preview
  * mouseovers and 2 to indicate permanent selection.
  * The latter is how modelbuilder uses SMTK selections.
  *
  * When an object is assigned the special integer 0,
  * the object is considered "unselected" and is removed
  * from the map.
  *
  * ## Lifecycle
  *
  * First, your application should create a selection instance for
  * each separate context in which a separate selection is allowed.
  * If your application will be making changes to resources by
  * invoking operators, you should register the operation manager with
  * the selection so that as persistent objects are created and
  * destroyed the selection can be updated.
  *
  * After creating a selection, each UI element of your
  * application that will (a) share the selection and (b) modify
  * the selection should register itself as a source of selections
  * by providing a unique string name to registerSelectionSource().
  *
  * Similarly, each UI element of your application that will (a) share
  * the selection and (b) present the selection or otherwise need to
  * be informed of changes to the selection should register as an
  * observer by providing a callback to observe().
  *
  * ## Selection Events
  *
  * When the selection changes, Observer functions subscribed to
  * updates will be called with the name of the UI elements that
  * caused the event (or "selection" if a change inside the
  * selection itself caused the event). A pointer to the selection
  * is also provided so you can query the selection
  * inside the Observer function. You should **not** modify
  * the selection inside a Observer as that can cause infinite
  * recursion.
  *
  * The Observer may be called under a variety of circumstances:
  *
  * + upon initial registration with the selection (if
  *   \a immediatelyNotify is passed as true),
  * + upon modification by a selection source
  * + upon a change to the selection's filter that
  *   results in a change to the selection.
  *
  * While Observers are not usually informed when attempted
  * changes to the selection have no effect, it is possible
  * to get called when the selection is entirely replaced with
  * an identical selection.
  *
  * ## Convenience Functions
  *
  * Often, observers may wish to make use of an updated selection
  * by applying it to an attribute ReferenceItem (and particularly
  * when a ReferenceItem is an operation-parameter's associations).
  * The selection provides the configureItem() method to support
  * this use case.
  */
class SMTKCORE_EXPORT Selection : smtkEnableSharedPtr(Selection)
{
public:
  using Observer = SelectionObserver;
  using Observers = SelectionObservers;
  using Component = smtk::resource::Component;
  using Object = smtk::resource::PersistentObject;

  smtkTypeMacroBase(Selection);
  smtkCreateMacro(Selection);

  Selection(const Selection&) = delete;
  Selection& operator=(const Selection&) = delete;

  static Ptr instance();

  /// This is the underlying storage type that holds selections.
  using SelectionMap = std::map<Object::Ptr, int>;

  /**\brief Selection filters take functions of this form.
    *
    * Given an object and its selection "value", return true if
    * the object should be included in the action (replacement,
    * addition to, or removal from the selection). Otherwise return false.
    *
    * The filter may also insert objects into the map (the 3rd argument)
    * which will unconditionally be included in the action.
    * This is intended to handle use cases where related objects
    * (e.g., edges, faces, vertices) may be geometrically picked by a user
    * but the desired selection is on objects not directly rendered
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
    * Selected objects each take on a non-zero integer value that is application-defined.
    *
    * The methods in this section are used to register particular values or bits that the
    * application wishes to use. Each value is tied to a string name.
    */
  //@{
  /// Register a selection value. If the value was already registered, returns false.
  bool
  registerSelectionValue(const std::string& valueLabel, int value, bool valueMustBeUnique = true);
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
    * \a objects should replace the selection, be added to it, or be removed from it.
    * \a value indicates the "level" at which the \a objects should be selected
    * (except when the \a action is a subtraction operation, in which case \a value is ignored).
    * \a source indicates which UI element is responsible for the selection modification.
    * \a bitwise indicates whether, treating \a value as a bit-vector, the
    * selection should only modify those bits.
    *
    * When \a bitwise is true, then actions which ADD to the selection will
    * insert new entries into the selection with \a value bits set unless those
    * objects were already present, in which case their values will be OR-ed with
    * \a value.
    * Actions which SUBTRACT from the selection will remove entries whose bits are
    * a strict subset of those in \a value; otherwise objects will have \a value's
    * bits removed from the selection map but will remain in the map.
    * Actions which REPLACE the selection will remove any objects already in
    * the selection map whose bits are a subset of \a value, insert objects
    * not already present in the map (with the given \a value), and OR \a value
    * into the bits of any present in both the selection and the "replacement"
    * objects.
    *
    * If \a bitwise is false, then actions which ADD to the selection will
    * overwrite any existing selection-map entry for matching objects with the
    * given \a value.
    * Actions which SUBTRACT from the selection, will remove entries from the
    * selection only when their mapped value exactly matches \a value.
    * Actions which REPLACE the selection will erase the entire selection map and
    * then insert the provided \a objects with the given \a value.
    */
  template<typename T>
  bool modifySelection(
    const T& objects,
    const std::string& source,
    int value,
    SelectionAction action = SelectionAction::DEFAULT,
    bool bitwise = false,
    bool postponeNotification = false);

  /**\brief Reset values in the selection map so no entries contain the given bit \a value.
    *
    * This method assumes each value in the selection map is a bit vector.
    *
    * This will remove objects from the selection entirely if their
    * selection value is a subset of the bits set in \a value.
    */
  bool resetSelectionBits(const std::string& source, int value);

  /**\brief Default selection action.
    *
    * Some applications need to separate the choice of which SelectionAction
    * to use from the choice of objects in the selection.
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
    * You can obtain the current selection in bulk or objects selected
    * with a particular value.
    * If you are using selection values as independent bits in a it vector,
    * pass \a exactMatch = false.
    */
  //@{
  /// Visit every selected object with the given functor.
  void visitSelection(std::function<void(Object::Ptr, int)> visitor)
  {
    for (const auto& entry : m_selection)
    {
      visitor(entry.first, entry.second);
    }
  }
  /// Return the current selection as a map from objects to integer selection values.
  SelectionMap& currentSelection(SelectionMap& selection) const;
  const SelectionMap& currentSelection() const { return m_selection; }
  /// Return the subset of selected elements that match the given selection value.
  template<typename T>
  T& currentSelectionByValue(T& selection, int value, bool exactMatch = true) const;
  template<typename T>
  T& currentSelectionByValue(T& selection, const std::string& valueLabel, bool exactMatch = true)
    const;

  template<typename T>
  T currentSelectionByValueAs(int value, bool exactMatch = true) const
  {
    T result;
    this->currentSelectionByValue(result, value, exactMatch);
    return result;
  }
  template<typename T>
  T currentSelectionByValueAs(const std::string& valueLabel, bool exactMatch = true) const
  {
    T result;
    this->currentSelectionByValue(result, valueLabel, exactMatch);
    return result;
  }
  //@}

  /// Return the observers associated with this phrase model.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /** \brief Selection filtering.
    *
    */
  //@{
  /// Set the filter to apply to each persistent object.
  void setFilter(const SelectionFilter& fn, bool refilterSelection = true);
  //@}

  /**\brief Convenience functions.
    *
    */
  //@{
  /**\brief Replace the reference item's membership with values from the selection.
    *
    * Objects in the currentSelection() are added to the item when
    *
    * + their value in the selection map matches the provided \a value
    *   (exactly when \a exactMatch is true or has all of \a value's
    *   bits set otherwise).
    * + the object is accepted by the item (i.e., it matches one or more of the
    *   filters returned by the item's acceptableEntries() method).
    *
    * By default, the \a item's pre-existing membership is cleared before the
    * selection is added to the \a item. (This can be changed by passing
    * false to \a clearItem.)
    *
    * If the \a item is optional, it will be disabled if no entries in the selection
    * match and (if \a clearItem is false) there are no pre-existing entries
    * retained. Otherwise, the \a item will be enabled. Note that if no values
    * in the \a item are modified, no change will be made to the item's isEnabled()
    * status.
    *
    * This method returns true if the item's membership was changed and false
    * otherwise. If true, you may wish to run the smtk::attribute::Signal
    * operation so that user interface components can be properly notified.
    */
  bool configureItem(
    const std::shared_ptr<smtk::attribute::ReferenceItem>& item,
    int value,
    bool exactMatch = false,
    bool clearItem = true) const;
  //@}

protected:
  Selection();

  /// Perform the action (**ignoring** m_defaultAction!!!), returning true if it had an effect.
  bool performAction(
    smtk::resource::PersistentObjectPtr comp,
    int value,
    SelectionAction action,
    SelectionMap& suggested,
    bool bitwise);
  bool refilter(const std::string& source);

  SelectionAction m_defaultAction{ SelectionAction::FILTERED_REPLACE };
  //smtk::model::BitFlags m_modelEntityMask;
  bool m_meshSetMask;
  std::set<std::string> m_selectionSources;
  std::map<std::string, int> m_selectionValueLabels;
  SelectionMap m_selection;
  Observers m_observers;
  SelectionFilter m_filter;
};

template<typename T>
T& Selection::currentSelectionByValue(T& selection, int value, bool exactMatch) const
{
  if (exactMatch)
  {
    for (const auto& entry : m_selection)
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
    for (const auto& entry : m_selection)
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

template<typename T>
T& Selection::currentSelectionByValue(T& selection, const std::string& valueLabel, bool exactMatch)
  const
{
  int val = this->selectionValueFromLabel(valueLabel);
  return this->currentSelectionByValue(selection, val, exactMatch);
}

template<typename T>
bool Selection::modifySelection(
  const T& objects,
  const std::string& source,
  int value,
  SelectionAction action,
  bool bitwise,
  bool postponeNotification)
{
  bool modified = false;
  SelectionMap suggestions;
  if (action == SelectionAction::DEFAULT)
  {
    action = this->defaultAction();
  }
  if (
    !bitwise &&
    (action == SelectionAction::FILTERED_REPLACE || action == SelectionAction::UNFILTERED_REPLACE))
  {
    modified = !m_selection.empty();
    m_selection.clear();
  }
  else if (
    bitwise &&
    (action == SelectionAction::FILTERED_REPLACE || action == SelectionAction::UNFILTERED_REPLACE))
  {
    // Remove unmatched objects from existing selection
    int mask = ~value;
    smtk::resource::PersistentObjectSet willErase;
    for (auto& entry : m_selection)
    {
      auto it = std::find(objects.begin(), objects.end(), entry.first);
      // std::cout << "  " << entry.first->id() << "  " << (it == objects.end() ? "not in obj" : "in obj") << " " << entry.second << " " << (entry.second & mask);
      if ((it == objects.end() && ((entry.second & mask) == 0)) || value == 0)
      {
        willErase.insert(entry.first);
        // std::cout << "    erase obj\n";
      }
      else if (it == objects.end() && ((entry.second & mask) != 0))
      {
        entry.second &= mask;
        modified = true;
      }
      else
      {
        // std::cout << "    keep  obj\n";
      }
    }
    // std::cout << "---\n";
    if (!willErase.empty())
    {
      modified = true;
      for (const auto& key : willErase)
      {
        m_selection.erase(key);
      }
    }
  }
  for (const auto& object : objects)
  {
    modified |= this->performAction(object, value, action, suggestions, bitwise);
  }
  if (modified && !postponeNotification)
  {
    this->observers()(source, shared_from_this());
  }
  return modified;
}

} // namespace view
} // namespace smtk

#endif
