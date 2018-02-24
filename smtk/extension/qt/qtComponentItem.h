//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtComponentItem_h
#define smtk_extension_qt_qtComponentItem_h

#include "smtk/extension/qt/qtReferenceItem.h"

#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class QBoxLayout;

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtComponentItem : public qtReferenceItem
{
  Q_OBJECT
  using Superclass = qtReferenceItem;

public:
  qtComponentItem(smtk::attribute::ComponentItemPtr, QWidget* p, qtBaseView* bview,
    Qt::Orientation enumOrient = Qt::Horizontal);
  virtual ~qtComponentItem();

  void setLabelVisible(bool) override;
  smtk::attribute::ComponentItemPtr componentItem();

protected slots:
  void updateItemData() override;

protected:
  void createWidget() override;

  class Internal;
  Internal* m_p;
};
}
}
#endif
