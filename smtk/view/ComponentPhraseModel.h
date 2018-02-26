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

  static PhraseModelPtr create(const ViewPtr& view);

protected:
  ComponentPhraseModel();

  /*
  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln);
  virtual void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event);
  virtual int handleOperationEvent(Operation::Ptr op, Operator::EventType event, Operator::Result res);

  virtual void handleExpunged(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  virtual void handleModified(Operation::Ptr op, Operation::Result res, ComponentItemPtr data);
  */
  void handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event) override;
  void handleCreated(Operation::Ptr op, Operation::Result res, ComponentItemPtr data) override;

  virtual void processResource(Resource::Ptr rsrc, bool adding);

  smtk::view::DescriptivePhrasePtr m_root;
  std::set<smtk::resource::ResourcePtr> m_resources;
  bool m_singleResource;
};
}
}

#endif
