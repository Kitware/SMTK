//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtGroupItem - UI components for attribute GroupItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtGroupItem_h
#define __smtk_attribute_qtGroupItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"

class qtGroupItemInternals;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtGroupItem : public qtItem
    {
      Q_OBJECT

    public:
      qtGroupItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview,
                  Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      virtual ~qtGroupItem();
      virtual void setLabelVisible(bool);

    protected slots:
      virtual void updateItemData();
      virtual void setEnabledState(bool checked);
      virtual void onAddSubGroup();
      virtual void onRemoveSubGroup();
      virtual void onChildWidgetSizeChanged();

    protected:
      virtual void createWidget();
      virtual void addSubGroup(int i);
      virtual void updateExtensibleState();
      virtual void addItemsToTable(int i);

    private:
      qtGroupItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
