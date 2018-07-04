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
  using Operation = smtk::operation::Operation;

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
  virtual int handleOperationEvent(Operation::Ptr op, Operator::EventType event, Operator::Result res);
  */
  void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event) override;
  void handleCreated(Operation::Ptr op, Operation::Result res, ComponentItemPtr data) override;
  void handleModified(Operation::Ptr op, Operation::Result res, ComponentItemPtr data) override;
  void handleExpunged(Operation::Ptr op, Operation::Result res, ComponentItemPtr data) override;

  virtual void processResource(Resource::Ptr rsrc, bool adding);

  smtk::view::DescriptivePhrasePtr m_root;
  std::set<smtk::resource::ResourcePtr> m_resources;
};
}
}

#endif
