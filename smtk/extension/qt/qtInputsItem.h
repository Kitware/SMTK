//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtInputsItem - an analysis type item
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtInputsItem_h
#define __smtk_attribute_qtInputsItem_h

#include "smtk/extension/qt/qtItem.h"

class qtInputsItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtInputsItem : public qtItem
    {
      Q_OBJECT

    public:
      qtInputsItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtInputsItem();
      virtual void setLabelVisible(bool);

    public slots:
      void setOutputOptional(int);

    protected slots:
      virtual void updateItemData();
      virtual void onAddNewValue();
      virtual void onRemoveValue();

    protected:
      virtual void createWidget();
      virtual void loadInputValues();
      virtual void updateUI();
      virtual void addInputEditor(int i);
      virtual void updateExtensibleState();
      virtual void clearChildWidgets();

    private:

      qtInputsItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
