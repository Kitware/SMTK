//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseModel_h
#define smtk_view_PhraseModel_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/Deprecation.h"
#include "smtk/common/ThreadPool.h"
#include "smtk/common/TypeContainer.h"
#include "smtk/common/Visit.h"

#include "smtk/view/BadgeSet.h"
#include "smtk/view/PhraseContent.h"
#include "smtk/view/PhraseModelObserver.h"
#include "smtk/view/Selection.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include <functional>
#include <list>
#include <set>
#include <unordered_map>

namespace smtk
{
namespace view
{

// Typedef representing a map of UUID to a set of weak pointers to Descriptive Phrases that refer to
// that UUId's PersistentObject
typedef std::unordered_map<
  smtk::common::UUID,
  std::set<
    std::weak_ptr<smtk::view::DescriptivePhrase>,
    std::owner_less<std::weak_ptr<smtk::view::DescriptivePhrase>>>>
  UUIDsToPhrasesMap;

// Sort paths from deepest to shallowest, then rear-most to front-most.
// Doing these things keeps us from invalidating paths when items are removed.
struct PathComp
{
  bool operator()(const std::vector<int>& a, const std::vector<int>& b) const
  {
    if (a.size() < b.size())
    {
      return false;
    }
    else if (a.size() > b.size())
    {
      return true;
    }
    std::size_t ii = 0;
    for (auto ai : a)
    {
      if (ai < b[ii])
      {
        return false;
      }
      else if (ai > b[ii])
      {
        return true;
      }
      ++ii;
    }
    return false; // a == b... neither is less than other.
  }
};

class PhraseDeltas : public std::set<std::vector<int>, PathComp>
{
};

/**\brief Hold and maintain a descriptive phrase hierarchy.
  *
  * This class holds the root descriptive phrase in a hierarchy and
  * tracks changes in SMTK resources in order to keep the hierarchy up to date.
  *
  * In the model-view-controller pattern, this class is a model -- hence its name.
  * It is a model that maintains a descriptive phrase hierarchy for some
  * particular purpose: subclasses will determine that purpose, whether it
  * be to illustrate the entire set of resources in memory, to show the current
  * selection, to preview changes associated with an operation, etc.
  *
  * This class accepts observers for events that result in changes to the
  * hierarchy so that user-interface-specific code can present data in this model.
  */
class SMTKCORE_EXPORT PhraseModel : smtkEnableSharedPtr(PhraseModel)
{
public:
  /// Events that alter the phrase model trigger callbacks of this type.
  using Observer = PhraseModelObserver;
  /// A collection of observers with runtime-modifiable call behaviors.
  using Observers = PhraseModelObservers;
  /// Applications may have a model decorate its phrases by providing a method with this signature.
  using PhraseDecorator = std::function<void(smtk::view::DescriptivePhrasePtr)>;
  /// Subclasses (and others) may wish to invoke functions on the sources of data for the phrases.
  /// Note that this is an old format that is being deprecated.
  using SourceVisitor = std::function<bool(
    const smtk::resource::ManagerPtr&,
    const smtk::operation::ManagerPtr&,
    const smtk::view::ManagerPtr&,
    const smtk::view::SelectionPtr&)>;

  /// Subclasses (and others) may wish to invoke functions on the sources of data for the phrases.
  /// Note that this is an current format that is being supported.
  using SourceVisitorFunction = std::function<smtk::common::Visit(
    const smtk::resource::ManagerPtr&,
    const smtk::operation::ManagerPtr&,
    const smtk::view::ManagerPtr&,
    const smtk::view::SelectionPtr&)>;

  using Operation = smtk::operation::Operation;
  using OperationPtr = smtk::operation::Operation::Ptr;
  using ComponentItemPtr = smtk::attribute::ComponentItemPtr;

  using Resource = smtk::resource::Resource;

  smtkTypeMacroBase(smtk::view::PhraseModel);
  smtkCreateMacro(smtk::view::PhraseModel);
  virtual ~PhraseModel();

  /// A method subclasses may call to prepare a subphrase generator.
  ///
  /// PhraseModel's constructor does not call it since it does not
  /// create the root DescriptivePhrase to which the subphrase generator
  /// is attached.
  static SubphraseGeneratorPtr configureSubphraseGenerator(const Configuration*, Manager*);

  /// A method subclasses may call to obtain filter strings.
  static std::multimap<std::string, std::string> configureFilterStrings(
    const Configuration* config,
    Manager* manager);

  /** \brief Manage sources of information to display as phrases.
    *
    * These methods provide a way for the phrase model to stay informed
    * of changes to resources in SMTK.
    * Adding sources results in callbacks being registered;
    * then, when SMTK resources are modified, the different handleXXX()
    * methods are invoked so that subclasses of PhraseModel can update
    * the phrase hierarcy as required.
    */
  ///@{
  /// Indicate the managers that should be monitored for changes.
  virtual bool addSource(const smtk::common::TypeContainer& managers);
  /// Indicate managers that should no longer be monitored for changes.
  virtual bool removeSource(const smtk::common::TypeContainer& managers);
  /// Stop listening for changes from all sources.
  virtual bool resetSources();
  /// Invoke the visitor function on each source that has been added to the model.
  virtual void visitSources(SourceVisitorFunction visitor);
  /**\brief The priority used for observing operation results.
    *
    * Since other application UI elements may need to observe operations
    * and modify the phrase model (e.g., to change the selection or
    * expand/collapse subtrees), this class uses a priority that is higher
    * than the default. Your application can then ensure its observer(s)
    * run at a lower priority if it needs phrases updated before it runs.
    */
  static smtk::operation::Observers::Priority operationObserverPriority();
  ///@}

  /// Return the root phrase of the hierarchy.
  virtual DescriptivePhrasePtr root() const;

  /**\brief Infer changes between sets of subphrases; affect these changes; and notify observers.
    */
  virtual void updateChildren(
    smtk::view::DescriptivePhrasePtr plist,
    DescriptivePhrases& next,
    const std::vector<int>& idx);

  /// Manually specify that all rows should be updated (but to keep the expanded/collapsed state).
  virtual void triggerDataChanged();

  /// Manually specify that all rows with their relatedComponent() == \a comp should be updated.
  virtual void triggerDataChangedFor(smtk::resource::ComponentPtr comp);

  /// Return the observers associated with this phrase model.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Return the operationManager for the first Source registered
  smtk::operation::ManagerPtr operationManager() const;

  /**\brief Set what aspects of top-level phrases should be user-editable.
    *
    * Note that this does not specify the means by which the
    * editing will occur; it is merely a guide that the UI layers
    * which own/use instances of this class may use to determine
    * which editing actions to make available.
    *
    * Valid bits are from the PhraseContent::ContentType enum.
    * By default, this is PhraseContent::EVERYTHING.
    */
  void setMutableAspects(int mutableAspects) { m_mutableAspects = mutableAspects; }

  /**\brief Get what aspects of top-level phrases should be user-editable.
    *
    * Valid bits are from the PhraseContent::ContentType enum.
    */
  int mutableAspects() const { return m_mutableAspects; }

  /// PhraseModels that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return m_manager.lock(); }
  friend Manager;

  /// Return the badges that may apply to phrases in this model.
  const BadgeSet& badges() const { return m_badges; }
  BadgeSet& badges() { return m_badges; }

  /// Return the map between persistent object IDs and Descriptive Phrases
  const UUIDsToPhrasesMap uuidPhraseMap() const { return m_objectMap; }

  smtk::common::ThreadPool<DescriptivePhrases>& threadPool() { return m_pool; }

protected:
  PhraseModel();
  PhraseModel(const Configuration* config, Manager* manager);

  /// A method called when a selection is modified.
  virtual void handleSelectionEvent(const std::string& src, smtk::view::SelectionPtr seln);

  /// A method called when a resource manager adds or removes a resource.
  ///
  /// By default, this does nothing. Subclasses may override it if needed, but
  /// most resource events should be handled by observing operations that act
  /// on resources rather than by observing the resource manager.
  virtual void handleResourceEvent(const Resource&, smtk::resource::EventType) {}

  /**\brief A method called when operators have modified one or more resources.
    *
    * You may subclass this method or, perhaps preferrably, one of the specific methods
    * that this method invokes (handleExpunged, handleModified, handleCreated).
    * The only case where overriding this method is required (as opposed to those listed above)
    * is when you wish to respond to events other than Operation::DID_OPERATE.
    */
  virtual int
  handleOperationEvent(const Operation& op, operation::EventType e, const Operation::Result& res);

  /// This is invoked from within handleOperationEvent when a resource is being added or removed.
  ///
  /// The base implementation does nothing; it is up to subclasses to implement this as needed.
  ///
  /// When \a adding is true, an operation has created \a resource and indicated it should be
  /// managed by inserting it into the result's "resource" ResourceItem.
  /// When \a adding is false, an operation has requested that \a resource should be removed
  /// from its resource manager by inserting it into the result's "resourcesToExpunge"
  /// ResourceItem.
  virtual void processResource(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    bool adding);

  /**\brief Given the index of parent phrase and a range of its children, delete them.
    *
    * This properly signals any observers as it removes the specified children.
    * It is called from handleExpunged.
    */
  void removeChildren(const std::vector<int>& parentIdx, int childRange[2]);

  /**\brief Set the Parent of a Descriptive Phrase */
  void setPhraseParent(const DescriptivePhrasePtr& phrase, const DescriptivePhrasePtr& parent)
    const;

  /// Called to deal with resources/components being removed as a result of an operation.
  virtual void handleExpunged(const smtk::resource::PersistentObjectSet& expungedObjects);
  /// Called to deal with resources/components marked as modified by the operation.
  virtual void handleModified(const smtk::resource::PersistentObjectSet& modifiedObjects);
  /// Called to deal with resources/components being created as a result of an operation.
  virtual void handleCreated(const smtk::resource::PersistentObjectSet& createdObjects);

  /**\brief Un-decorate and re-decorate every phrase in the current hierarchy.
    *
    * This is called by setDecorator() to ensure that phrases which were
    * prepared before the current decorator are properly prepared.
    *
    * To un-decorate, all but the final PhraseContent object are popped from
    * each phrase. Then, the phrase-model's current decorator is invoked on
    * every phrase.
    *
    * This process assumes that if you have multiple decorator functions,
    * they chain each other.
    */
  virtual void redecorate();

  /** \brief Make changes to the phrase hierarchy.
    *
    * These methods may be used inside your subclass's overrides of the handleXXX() calls
    * in order to trigger the proper events for GUI elements watching the PhraseModel.
    */
  ///@{
  /// Modify the children of \a plist to be \a next; \a plist is located at \a idx in the hierarchy.
  ///@}

  /// "Emit" an event (by calling all observers with the given parameters)
  virtual void trigger(
    DescriptivePhrasePtr phr,
    PhraseModelEvent e,
    const std::vector<int>& src,
    const std::vector<int>& dst,
    const std::vector<int>& refs);

  /**\brief Remove the Descriptive Phrase and its descendants from the Persistent Object / Phrases Map
   */
  void removeFromMap(const DescriptivePhrasePtr& phr);

  /**\brief Insert the Descriptive Phrase and its descendants into the Persistent Object / Phrases Map
   */
  void insertIntoMap(const DescriptivePhrasePtr& phrase);

  struct Source
  {
    smtk::common::TypeContainer m_managers;
    smtk::resource::Observers::Key m_rsrcHandle;
    smtk::operation::Observers::Key m_operHandle;
    smtk::view::SelectionObservers::Key m_selnHandle;
    Source() = default;

    [[deprecated("PhraseModel::Source::Source now accepts managers held in a const "
                 "smtk::common::TypeContainer&")]] Source(smtk::resource::ManagerPtr rm, smtk::operation::ManagerPtr om, smtk::view::ManagerPtr vm, smtk::view::SelectionPtr sn, smtk::resource::Observers::Key&& rh, smtk::operation::Observers::Key&& oh, smtk::view::SelectionObservers::Key&& sh);

    Source(
      const smtk::common::TypeContainer& managers,
      smtk::resource::Observers::Key&& rh,
      smtk::operation::Observers::Key&& oh,
      smtk::view::SelectionObservers::Key&& sh);
  };
  std::list<Source> m_sources;

  UUIDsToPhrasesMap m_objectMap;
  Observers m_observers;

  PhraseDecorator m_decorator;

  BadgeSet m_badges;

  int m_mutableAspects{ PhraseContent::EVERYTHING };

  WeakManagerPtr m_manager;

  // Indicates we are in the process of updating children phrases
  bool m_updatingChildren = false;

  smtk::common::ThreadPool<DescriptivePhrases> m_pool;
};
} // namespace view
} // namespace smtk

#endif
