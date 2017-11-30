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
  using Operator = smtk::operation::Operator;

  smtkTypeMacro(ResourcePhraseModel);
  smtkSuperclassMacro(PhraseModel);
  smtkSharedPtrCreateMacro(PhraseModel);
  virtual ~ResourcePhraseModel();

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

  static PhraseModelPtr create(const ViewPtr& view);

protected:
  ResourcePhraseModel();

  /*
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);
  virtual void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event);
  virtual int handleOperatorEvent(Operator::Ptr op, Operator::EventType event, Operator::Result res);

  virtual void handleExpunged(Operator::Ptr op, Operator::Result res, ComponentItemPtr data);
  virtual void handleModified(Operator::Ptr op, Operator::Result res, ComponentItemPtr data);
  */
  void handleCreated(Operator::Ptr op, Operator::Result res, ComponentItemPtr data) override;

  smtk::view::PhraseListPtr m_root;
  std::set<smtk::resource::ResourcePtr> m_resources;
};
}
}

#endif
