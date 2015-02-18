//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelEntityItem - UI components for attribute ModelEntityItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtModelEntityItem_h
#define __smtk_attribute_qtModelEntityItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class qtModelEntityItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtModelEntityItem : public qtItem
    {
      Q_OBJECT

    public:
      qtModelEntityItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtModelEntityItem();
      virtual void setLabelVisible(bool);
      virtual void associateSelectedEntities(
          const smtk::common::UUIDs& selEntityRefs,
          bool resetExisting = true);

    public slots:
      void setOutputOptional(int);

    signals:
      void entitySelectionRequested(
        smtk::model::BitFlags entMask);

    protected slots:
      virtual void updateItemData();
      virtual void requestSelectionLink();

    protected:
      virtual void createWidget();
      virtual void loadAssociatedEntities();
      virtual void updateUI();
      virtual void addEntityAssociationWidget();

    private:

      qtModelEntityItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
