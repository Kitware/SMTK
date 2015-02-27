//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtMeshSelectionItem - UI components for attribute MeshSelectionItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtMeshSelectionItem_h
#define __smtk_attribute_qtMeshSelectionItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags
#include <set>

class qtMeshSelectionItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtMeshSelectionItem : public qtItem
    {
      Q_OBJECT

    public:
      qtMeshSelectionItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtMeshSelectionItem();
      virtual void setLabelVisible(bool);
      virtual void setSelection(const smtk::common::UUID& entid,
                                const std::set<int> vals);

      smtk::attribute::ModelEntityItemPtr refModelEntityItem();
      void setUsingCtrlKey(bool);
      bool usingCtrlKey();

    public slots:
      void setOutputOptional(int);

    signals:
      void requestMeshSelection(smtk::attribute::ModelEntityItemPtr pEntItem);

    protected slots:
      virtual void updateItemData();
      virtual void onRequestMeshSelection();

    protected:
      virtual void createWidget();
      virtual void updateUI();
      virtual void addMeshOpButtons();

    private:

      qtMeshSelectionItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
