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

#include "smtk/view/View.h"

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

  smtkTypeMacro(ResourcePhraseModel);
  smtkSuperclassMacro(PhraseModel);
  smtkSharedPtrCreateMacro(PhraseModel);
  virtual ~ResourcePhraseModel();

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

  static PhraseModelPtr create(const ViewPtr& view);
  static PhraseModelPtr create(const View::Component& view);

  bool setResourceFilters(const std::multimap<std::string, std::string>& src);

protected:
  ResourcePhraseModel();

  /*
  void handleSelectionEvent(const std::string& src, Selection::Ptr seln) override;
  void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event) override;
  int handleOperationEvent(smtk::operation::Operation::Ptr op, smtk::operation::EventType event,
    smtk::operation::Operation::Result res) override;
  */
  void handleResourceEvent(const Resource::Ptr& rsrc, smtk::resource::EventType event) override;

  virtual void processResource(const Resource::Ptr& rsrc, bool adding);
  virtual void triggerModified(const Resource::Ptr& rsrc);

  smtk::view::DescriptivePhrasePtr m_root;
  std::set<smtk::resource::ResourcePtr> m_resources;
  std::multimap<std::string, std::string> m_resourceFilters;
};
}
}

#endif
