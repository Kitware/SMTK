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

#include "smtk/view/Selection.h"

#include "smtk/resource/Event.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operator.h"

#include <functional>
#include <list>

#define smtkImplementsPhraseModel(exportMacro, className, compName, viewName)                      \
  /* Implement autoinit methods */                                                                 \
  void exportMacro smtk_##compName##_phrase_model_AutoInit_Construct()                             \
  {                                                                                                \
    smtk::view::PhraseModel::registerModelType(                                                    \
      #viewName, [](const smtk::view::ViewPtr& view) { return className::create(view); });         \
  }                                                                                                \
  void exportMacro smtk_##compName##_phrase_model_AutoInit_Destruct()                              \
  {                                                                                                \
    smtk::view::PhraseModel::unregisterModelType(#viewName);                                       \
  }

namespace smtk
{
namespace view
{

/// Events that can be observed on an smtk::view::PhraseModel.
enum class PhraseModelEvent
{
  ABOUT_TO_INSERT, //!< A phrase or range of phrases is about to be inserted in the parent.
  INSERT_FINISHED, //!< A phrase of range of phrases has been inserted in the parent.
  ABOUT_TO_REMOVE, //!< A phrase or range of phrases is about to be removed from the parent.
  REMOVE_FINISHED, //!< A phrase or range of phrases has been removed from the parent.
  ABOUT_TO_MOVE,   //!< A phrase or range of phrases is being moved from one place to another.
  MOVE_FINISHED,   //!< A phrase or range of phrases has been moved and the update is complete.
  PHRASE_MODIFIED  //!< The given phrase has had its text, color, or some other property modified.
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
  using Observer = std::function<void(DescriptivePhrasePtr, PhraseModelEvent,
    const std::vector<int>&, const std::vector<int>&, const std::vector<int>&)>;
  /// Subclasses of PhraseModel register their type information with a constructor of this type.
  using ModelConstructor = std::function<PhraseModelPtr(const ViewPtr&)>;
  /// Applications may have a model decorate its phrases by providing a method with this signature.
  using PhraseDecorator = std::function<void(smtk::view::DescriptivePhrasePtr)>;

  using Operator = smtk::operation::Operator;
  using OperatorPtr = smtk::operation::OperatorPtr;
  using ComponentItemPtr = smtk::attribute::ComponentItemPtr;

  using Resource = smtk::resource::Resource;
  using Selection = smtk::view::Selection;

  static bool registerModelType(const std::string& typeName, ModelConstructor);
  static bool unregisterModelType(const std::string& typeName);
  static PhraseModelPtr create(const smtk::view::ViewPtr& viewSpec);

  smtkTypeMacroBase(PhraseModel);
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
  bool addSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr);
  /// Indicate a resource and operation manager that should no longer be monitored for changes.
  bool removeSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr);
  /// Stop listening for changes from all sources.
  bool resetSources();
  ///@}

  /// Return the root phrase of the hierarchy.
  virtual DescriptivePhrasePtr root() const;

  /** \brief Manage links to user interfaces.
    *
    * These methods are called by GUI classes that wish to monitor
    * changes to the descriptive phrase hierarchy.
    */
  ///@{
  /// Add a listener to respond to changes this model makes to the descriptive phrase hierarchy. This may call subphrases().
  int observe(Observer obs, bool immediatelyNotify = true);
  /// Remove a listener added with observe() by passing the integer handle observe() returned.
  bool unobserve(int);
  ///@}

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

protected:
  PhraseModel();

  /// A method called when a selection is modified.
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);

  /// A method called when a resource manager adds or removes a resource.
  virtual void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event);

  /**\brief A method called when operators have modified one or more resources.
    *
    * You may subclass this method or, perhaps preferrably, one of the specific methods
    * that this method invokes (handleExpunged, handleModified, handleCreated).
    * The only case where overriding this method is required (as opposed to those listed above)
    * is when you wish to respond to events other than Operator::DID_OPERATE.
    */
  virtual int handleOperatorEvent(Operator::Ptr op, Operator::EventType e, Operator::Result res);

  /// Called to deal with resources/components being removed as a result of an operation.
  virtual void handleExpunged(Operator::Ptr op, Operator::Result res, ComponentItemPtr data);
  /// Called to deal with resources/components marked as modified by the operation.
  virtual void handleModified(Operator::Ptr op, Operator::Result res, ComponentItemPtr data);
  /// Called to deal with resources/components being created as a result of an operation.
  virtual void handleCreated(Operator::Ptr op, Operator::Result res, ComponentItemPtr data);

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
    int m_rsrcHandle;
    int m_operHandle;
    Source() {}
    Source(smtk::resource::ManagerPtr rm, smtk::operation::ManagerPtr om, int rh, int oh)
      : m_rsrcMgr(rm)
      , m_operMgr(om)
      , m_rsrcHandle(rh)
      , m_operHandle(oh)
    {
    }
  };
  std::list<Source> m_sources;

  std::map<int, Observer> m_observers;

  PhraseDecorator m_decorator;

  static std::map<std::string, ModelConstructor> s_modelTypes;
};
}
}

#endif
