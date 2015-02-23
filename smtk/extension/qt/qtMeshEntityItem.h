//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtMeshEntityItem - UI components for attribute MeshEntityItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtMeshEntityItem_h
#define __smtk_attribute_qtMeshEntityItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

class qtMeshEntityItemInternals;
class QBoxLayout;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtMeshEntityItem : public qtItem
    {
      Q_OBJECT

    public:

  /// Enumeration of mesh values modification type.
  enum MeshListUpdateType {
    RESET           = 0, //!< Reset the existing list)
    APPEND             , //!< Append to the existing list
    SUBTRACT           , //!< Subtract from existing list
    CANCEL               //!< Cancel current operation mode
  };

      qtMeshEntityItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtMeshEntityItem();
      virtual void setLabelVisible(bool);
      virtual void updateValues(const std::set<int> vals,
        MeshListUpdateType opType);

    public slots:
      void setOutputOptional(int);

    signals:
      void requestValuesUpdate(smtk::attribute::ModelEntityItemPtr pEntItem,
                               MeshListUpdateType opType);

    protected slots:
      virtual void updateItemData();
      virtual void onRequestValuesUpdate();

    protected:
      virtual void createWidget();
      virtual void updateUI();
      virtual void addMeshOpButtons();

    private:

      qtMeshEntityItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
