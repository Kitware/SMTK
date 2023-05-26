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

#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"

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

  typedef std::function<
    bool(const smtk::view::DescriptivePhrasePtr& a, const smtk::view::DescriptivePhrasePtr& b)>
    SortingCompFunc;

  smtkTypeMacro(smtk::view::ComponentPhraseModel);
  smtkSuperclassMacro(smtk::view::PhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);

  ComponentPhraseModel();
  ComponentPhraseModel(const Configuration*, Manager* mgr);
  ~ComponentPhraseModel() override;

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

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
    * If \a fn returns a non-zero value, then the visitation terminates early and \a fn will
    * not be called again.
    * Otherwise, iteration continues.
    */
  void visitComponentFilters(std::function<int(const std::string&, const std::string&)> fn) const;

  /**\brief sets a custom comparison function to sort descriptive phrases. This function is used
    * to sort nodes at all levels recursively.
    * Developers can use comparison functions defined in DescriptivePhrase to set this.
    * If this function is not called, the default value is DescriptivePhrase::compareByTypeThenTitle.
    * This function calls populateRoot() so sorting would be triggered immediately.
    */
  void setSortFunction(const SortingCompFunc& comparator);

protected:
  /*
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);
  virtual void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event);
  virtual int handleOperationEvent(Operation::Ptr op, Operator::EventType event, Operator::Result res);

  virtual void handleExpunged(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  virtual void handleModified(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  */
  void handleCreated(const smtk::resource::PersistentObjectSet& createdObjects) override;

  void processResource(const Resource::Ptr& rsrc, bool adding) override;
  virtual void populateRoot();
  /**\brief creates a DescriptivePhrase for  a top-level Component (i.e. one whose parent is root).
   * Returns null if comp would not be considered a top level phrase.
   */
  DescriptivePhrasePtr createTopPhrase(const smtk::resource::ComponentPtr& comp);

  smtk::view::DescriptivePhrasePtr m_root;
  std::set<
    std::weak_ptr<smtk::resource::Resource>,
    std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
    m_resources;
  std::multimap<std::string, std::string> m_componentFilters;

private:
  SortingCompFunc m_comparator = smtk::view::DescriptivePhrase::compareByTypeThenTitle;
};
} // namespace view
} // namespace smtk

#endif
