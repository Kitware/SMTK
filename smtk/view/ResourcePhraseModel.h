//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ResourcePhraseModel_h
#define smtk_view_ResourcePhraseModel_h

#include "smtk/view/PhraseModel.h"

#include "smtk/view/Configuration.h"

namespace smtk
{
namespace view
{

/**\brief Present phrases describing a set of resources held by one or more resource managers.
  *
  */
class SMTKCORE_EXPORT ResourcePhraseModel : public PhraseModel
{
public:
  using Observer = std::function<void(DescriptivePhrasePtr, PhraseModelEvent, int, int)>;
  using Operation = smtk::operation::Operation;

  smtkTypeMacro(smtk::view::ResourcePhraseModel);
  smtkSuperclassMacro(smtk::view::PhraseModel);
  smtkSharedPtrCreateMacro(smtk::view::PhraseModel);

  ResourcePhraseModel();
  ResourcePhraseModel(const Configuration*, Manager*);
  ~ResourcePhraseModel() override;

  static Ptr create(const Configuration*, Manager*);

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

  static PhraseModelPtr create(const smtk::view::ConfigurationPtr& view);
  static PhraseModelPtr create(const smtk::view::Configuration::Component& view);

  bool setResourceFilters(const std::multimap<std::string, std::string>& src);

  bool setFilter(std::function<bool(const smtk::resource::Resource&)>);
  const std::function<bool(const smtk::resource::Resource&)>& filter() const { return m_filter; }

protected:
  /*
  void handleSelectionEvent(const std::string& src, Selection::Ptr seln) override;
  */
  int handleOperationEvent(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    const smtk::operation::Operation::Result& res) override;
  void handleResourceEvent(const Resource& rsrc, smtk::resource::EventType event) override;

  void processResource(const Resource::Ptr& rsrc, bool adding) override;
  virtual void triggerModified(const Resource::Ptr& rsrc);

  smtk::view::DescriptivePhrasePtr m_root;
  std::function<bool(const smtk::resource::Resource&)> m_filter;
};
} // namespace view
} // namespace smtk

#endif
