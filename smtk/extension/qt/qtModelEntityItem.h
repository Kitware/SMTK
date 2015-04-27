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
#include "smtk/extension/qt/Exports.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class qtModelEntityItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtModelEntityItem : public qtItem
    {
      Q_OBJECT

    public:
      qtModelEntityItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtModelEntityItem();
      virtual void setLabelVisible(bool);
      virtual void associateEntities(
          const smtk::model::EntityRefs& selEntityRefs,
          bool resetExisting = true);

    public slots:
      void setOutputOptional(int);
      virtual void onRequestEntityAssociation();

    signals:
      void requestEntityAssociation();
      void entityListHighlighted(
        const smtk::common::UUIDs& uuids);

    protected slots:
      virtual void updateItemData();
      virtual void popupViewItemSelected();
      virtual void clearEntityAssociations();

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
