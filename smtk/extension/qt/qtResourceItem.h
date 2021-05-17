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
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtResourceItem(const qtAttributeItemInfo& info);
  ~qtResourceItem() override;

protected:
  smtk::view::PhraseModelPtr createPhraseModel() const override;

  std::string synopsis(bool& membershipValid) const override;
};
} // namespace extension
} // namespace smtk
#endif
