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
#include "smtk/extension/qt/Exports.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

#include <map>
#include <set>

class qtMeshSelectionItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtMeshSelectionItem : public qtItem
    {
      Q_OBJECT

    public:
      qtMeshSelectionItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtMeshSelectionItem();
      virtual void setLabelVisible(bool);
      // update the selection input to the operation
      virtual void updateInputSelection(
        const std::map<smtk::common::UUID, std::set<int> >& selectionValues);

      smtk::attribute::ModelEntityItemPtr refModelEntityItem();
      void setUsingCtrlKey(bool);
      bool usingCtrlKey();
      // update the cached selection, and return the result in outSelectionValues
      void syncWithCachedSelection(
        const smtk::attribute::MeshSelectionItemPtr& meshSelectionItem,
        std::map<smtk::common::UUID, std::set<int> > &outSelectionValues);

    public slots:
      void setOutputOptional(int);
      void clearSelection();
      void resetSelectionState();

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
