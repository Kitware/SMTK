//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ComponentPhraseModel_h
#define smtk_view_ComponentPhraseModel_h

#include "smtk/view/PhraseModel.h"
#include "smtk/view/View.h"

#include <functional>
#include <map>

namespace smtk
{
namespace view
{
/**\brief Present phrases describing a set of acceptable components held by a single resource.
  *
  * This model maintains the list of acceptable components by
  * asking the resource for all matching components each time
  * an operation runs.
  *
  * The list is flat (i.e., no subphrase generator is provided by default).
  * The model provides access to a user-selected subset as an smtk::view::Selection.
  */
class SMTKCORE_EXPORT ComponentPhraseModel : public PhraseModel
{
public:
  using Observer = std::function<void(DescriptivePhrasePtr, PhraseModelEvent, int, int)>;
  using Operation = smtk::operation::Operation;

  smtkTypeMacro(ComponentPhraseModel);
  smtkSuperclassMacro(PhraseModel);
  smtkSharedPtrCreateMacro(PhraseModel);
  virtual ~ComponentPhraseModel();

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

  /**\brief Create a model and configure it given a view description.
    *
    * Note that this method, unlike the version with no parameters,
    * properly initializes its subphrase generator with a reference to
    * the created model so that subphrases are properly decorated.
    */
  static PhraseModelPtr create(const View::Component& viewComp);

  /// Return the active resource (i.e., the one resource whose components should be displayed).
  smtk::resource::ResourcePtr activeResource() const { return m_activeResource; }

  /// Set the active resource (i.e., the one resource whose components should be displayed).
  bool setActiveResource(smtk::resource::ResourcePtr rsrc);

  /// Return whether this model should *only* display components from the active resource.
  bool onlyShowActiveResourceComponents() const { return m_onlyShowActiveResourceComponents; }

  /** Set whether this model should *only* display components from the active resource.
    *
    * The default is false (i.e., all matching components for every resource detected
    * from every source added (see addSource()) will be displayed).
    *
    * This returns true when the value was changed and false otherwise.
    */
  bool setOnlyShowActiveResourceComponents(bool limitToActiveResource);

  /**\brief Set the specification for what components are allowed to be \a src.
    *
    * Note that entries in \a src are tuples of (unique name, filter);
    * the unique name specifies a resource type (and all its subclasses) deemed acceptable sources
    * while the filter is a string that can be passed to any acceptable resource to get a
    * set of components it owns that are acceptable.
    * In other words, both the resource and a subset of its components must be acceptable.
    */
  bool setComponentFilters(const std::multimap<std::string, std::string>& src);

  /**\brief Visit all of the filters that define what components are acceptable for display.
    *
    * The function \a fn is called with each entry in m_componentFilters, with a resource's
    * unique name as the first argument and the component filter as the second argument.
    * If \a fn returns a non-zero value, then the visitation terminates early and \fn will
    * not be called again.
    * Otherwise, iteration continues.
    */
  void visitComponentFilters(std::function<int(const std::string&, const std::string&)> fn) const;

protected:
  ComponentPhraseModel();

  /*
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);
  virtual void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event);
  virtual int handleOperationEvent(Operation::Ptr op, Operator::EventType event, Operator::Result res);

  virtual void handleExpunged(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  virtual void handleModified(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  */
  void handleResourceEvent(const Resource::Ptr& rsrc, smtk::resource::EventType event) override;
  void handleCreated(
    const Operation::Ptr& op, const Operation::Result& res, const ComponentItemPtr& data) override;

  virtual void processResource(Resource::Ptr rsrc, bool adding);
  virtual void populateRoot();

  smtk::view::DescriptivePhrasePtr m_root;
  std::set<smtk::resource::ResourcePtr> m_resources;
  smtk::resource::ResourcePtr m_activeResource;
  bool m_onlyShowActiveResourceComponents;
  std::multimap<std::string, std::string> m_componentFilters;
};
}
}

#endif
