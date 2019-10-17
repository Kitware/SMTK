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

#include "smtk/view/PhraseModelObserver.h"
#include "smtk/view/Selection.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include <functional>
#include <list>

namespace smtk
{
namespace view
{

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
  /// Subclasses of PhraseModel register their type information with a constructor of this type.
  using ModelConstructor = std::function<PhraseModelPtr(const ViewPtr&)>;
  /// Applications may have a model decorate its phrases by providing a method with this signature.
  using PhraseDecorator = std::function<void(smtk::view::DescriptivePhrasePtr)>;

  using Operation = smtk::operation::Operation;
  using OperationPtr = smtk::operation::Operation::Ptr;
  using ComponentItemPtr = smtk::attribute::ComponentItemPtr;

  using Resource = smtk::resource::Resource;
  using Selection = smtk::view::Selection;

  static PhraseModelPtr create(
    const smtk::view::ViewPtr& viewSpec, const smtk::view::ManagerPtr& manager);

  smtkTypeMacroBase(smtk::view::PhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);
  virtual ~PhraseModel();

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
  /// Indicate a resource and operation manager that should be monitored for changes.
  bool addSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr,
    smtk::view::SelectionPtr seln);
  /// Indicate a resource and operation manager that should no longer be monitored for changes.
  bool removeSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr,
    smtk::view::SelectionPtr seln);
  /// Stop listening for changes from all sources.
  bool resetSources();
  ///@}

  /// Return the root phrase of the hierarchy.
  virtual DescriptivePhrasePtr root() const;

  /**\brief Infer changes between sets of subphrases; affect these changes; and notify observers.
    */
  virtual void updateChildren(
    smtk::view::DescriptivePhrasePtr plist, DescriptivePhrases& next, const std::vector<int>& idx);

  /**\brief Decorate a descriptive phrase's content with helpers for a particular user interface.
    *
    * Subphrase generators will call this method as they prepare phrases in a hierarchy.
    * Subclasses of PhraseModel may also call this method as they prepare top-level phrases.
    *
    * This is an opportunity for the phrase model (which will be instantiated to connect and
    * manage a GUI element to SMTK subsystem state) to add application-specific helpers to
    * phrases.
    * For example, ModelBuilder's "SMTK Resource" panel ties the visibility of model entities
    * in its tree view to their visibility in the active 3-D ParaView view.
    * It does this by adding a decorator that overrides the value of the VISIBILITY attribute
    * as well as the method used to change the visibility.
    *
    * This **must** be called on every descriptive phrase **before** updateChildren() is invoked
    * on its parent, because updateChildren invokes the equality operator (==), which will not
    * identify identical phrases properly if they aren't decorated identically.
    */
  void decoratePhrase(smtk::view::DescriptivePhrasePtr) const;

  /**\brief Applications may decorate a model's phrases by providing a function.
    *
    * This function is invoked by the decoratePhrase() method and must be set
    * before any phrases which might be decorated exist in the model.
    * Otherwise, methods that compare phrases for equality will fail.
    *
    * The default function does nothing and setDecorator will not accept a NULL
    * \a phraseDecorator argument (so that run-time validity checking can be avoided).
    */
  bool setDecorator(const PhraseDecorator& phraseDecorator);

  /// Provide access to the existing decorator. You may chain decorators if you are careful.
  PhraseDecorator decorator() const { return m_decorator; }

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

protected:
  friend class VisibilityContent;
  PhraseModel();

  /// A method called when a selection is modified.
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);

  /// A method called when a resource manager adds or removes a resource.
  virtual void handleResourceEvent(const Resource& rsrc, smtk::resource::EventType event);

  /**\brief A method called when operators have modified one or more resources.
    *
    * You may subclass this method or, perhaps preferrably, one of the specific methods
    * that this method invokes (handleExpunged, handleModified, handleCreated).
    * The only case where overriding this method is required (as opposed to those listed above)
    * is when you wish to respond to events other than Operation::DID_OPERATE.
    */
  virtual int handleOperationEvent(
    const Operation& op, operation::EventType e, const Operation::Result& res);

  /**\brief Given the index of parent phrase and a range of its children, delete them.
    *
    * This properly signals any observers as it removes the specified children.
    * It is called from handleExpunged.
    */
  void removeChildren(const std::vector<int>& parentIdx, int childRange[2]);

  /// Called to deal with resources/components being removed as a result of an operation.
  virtual void handleExpunged(
    const Operation& op, const Operation::Result& res, const ComponentItemPtr& data);
  /// Called to deal with resources/components marked as modified by the operation.
  virtual void handleModified(
    const Operation& op, const Operation::Result& res, const ComponentItemPtr& data);
  /// Called to deal with resources/components being created as a result of an operation.
  virtual void handleCreated(
    const Operation& op, const Operation::Result& res, const ComponentItemPtr& data);

  /** \brief Make changes to the phrase hierarchy.
    *
    * These methods may be used inside your subclass's overrides of the handleXXX() calls
    * in order to trigger the proper events for GUI elements watching the PhraseModel.
    */
  ///@{
  /// Modify the children of \a plist to be \a next; \a plist is located at \a idx in the hierarchy.
  ///@}

  /// "Emit" an event (by calling all observers with the given parameters)
  virtual void trigger(DescriptivePhrasePtr phr, PhraseModelEvent e, const std::vector<int>& src,
    const std::vector<int>& dst, const std::vector<int>& refs);

  struct Source
  {
    smtk::resource::ManagerPtr m_rsrcMgr;
    smtk::operation::ManagerPtr m_operMgr;
    smtk::view::SelectionPtr m_seln;
    smtk::resource::Observers::Key m_rsrcHandle;
    smtk::operation::Observers::Key m_operHandle;
    smtk::view::SelectionObservers::Key m_selnHandle;
    Source() {}
    Source(smtk::resource::ManagerPtr rm, smtk::operation::ManagerPtr om,
      smtk::view::SelectionPtr sn, smtk::resource::Observers::Key rh,
      smtk::operation::Observers::Key oh, smtk::view::SelectionObservers::Key sh)
      : m_rsrcMgr(rm)
      , m_operMgr(om)
      , m_seln(sn)
      , m_rsrcHandle(rh)
      , m_operHandle(oh)
      , m_selnHandle(sh)
    {
    }
  };
  std::list<Source> m_sources;

  Observers m_observers;

  PhraseDecorator m_decorator;

  int m_mutableAspects;

  WeakManagerPtr m_manager;
};
}
}

#endif
