//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtResourceItem_h
#define smtk_extension_qt_qtResourceItem_h

#include "smtk/extension/qt/qtReferenceItem.h"

#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class QBoxLayout;

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtResourceItem : public qtReferenceItem
{
  Q_OBJECT
  using Superclass = qtReferenceItem;

public:
  static qtItem* createItemWidget(const AttributeItemInfo& info);
  qtResourceItem(const AttributeItemInfo& info);
  virtual ~qtResourceItem();

  void setLabelVisible(bool) override;

protected slots:
  void updateItemData() override;

protected:
  smtk::view::PhraseModelPtr createPhraseModel() const override;

  void createWidget() override;

  std::string synopsis(bool& membershipValid) const override;

  int decorateWithMembership(smtk::view::DescriptivePhrasePtr phr) override;

  void toggleCurrentItem() override;

  bool synchronize(UpdateSource src) override;
};
}
}
#endif
