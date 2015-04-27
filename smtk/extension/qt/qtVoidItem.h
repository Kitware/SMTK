//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtVoidItem - a qt item for Void type attribute items
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtVoidItem_h
#define __smtk_attribute_qtVoidItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"

class qtVoidItemInternals;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtVoidItem : public qtItem
    {
      Q_OBJECT

    public:
      qtVoidItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview);
      virtual ~qtVoidItem();
      virtual void setLabelVisible(bool);

    public slots:
      void setOutputOptional(int);

    protected slots:
      virtual void updateItemData();

    protected:
      virtual void createWidget();

    private:

      qtVoidItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
