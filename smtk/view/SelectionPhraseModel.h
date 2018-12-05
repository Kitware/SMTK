//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_SelectionPhraseModel_h
#define smtk_view_SelectionPhraseModel_h

#include "smtk/view/PhraseModel.h"

namespace smtk
{
namespace view
{

/**\brief Present phrases describing a set of resources held by one or more resource managers.
  *
  */
class SMTKCORE_EXPORT SelectionPhraseModel : public PhraseModel
{
public:
  using Observer = std::function<void(DescriptivePhrasePtr, PhraseModelEvent, int, int)>;
  using Operation = smtk::operation::Operation;

  smtkTypeMacro(SelectionPhraseModel);
  smtkSuperclassMacro(PhraseModel);
  smtkSharedPtrCreateMacro(PhraseModel);
  static PhraseModelPtr create(const ViewPtr& view);
  virtual ~SelectionPhraseModel();

  /// Return the root phrase of the hierarchy.
  DescriptivePhrasePtr root() const override;

  /// Set which bits of the selection value this phrase model will present.
  void setSelectionBit(int selnBit) { m_selectionBit = selnBit; }
  /// Return which bits of the selection this phrase model listens to.
  int selectionBit() const { return m_selectionBit; }

protected:
  SelectionPhraseModel();

  virtual void handleSelectionEvent(const std::string& src, Selection::Ptr seln) override;

  void populateRoot(const std::string& src, Selection::Ptr seln);

  smtk::view::DescriptivePhrasePtr m_root;
  smtk::view::SelectionPtr m_selection;
  int m_selectionBit;
  int m_componentMutability;
  int m_resourceMutability;
};
}
}

#endif
