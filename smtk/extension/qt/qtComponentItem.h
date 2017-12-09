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

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class QBoxLayout;

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtComponentItem : public qtItem
{
  Q_OBJECT
  using Superclass = qtItem;

public:
  qtComponentItem(smtk::attribute::ItemPtr, QWidget* p, qtBaseView* bview,
    Qt::Orientation enumOrient = Qt::Horizontal);
  virtual ~qtModelEntityItem();

  void setLabelVisible(bool) override;
  smtk::attribute::ComponentItemPtr componentItem();

  virtual std::string selectionSourceName() const { return this->m_selectionSourceName; }

protected slots:
  void updateItemData() override;

protected:
  void createWidget() override;
};
}
}
#endif
