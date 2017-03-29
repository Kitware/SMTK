//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeRefItem - UI component for attribute RefItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtAttributeRefItem_h
#define __smtk_extension_qtAttributeRefItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QComboBox>

class qtAttributeRefItemInternals;

namespace smtk
{
  namespace extension
  {
    class qtBaseView;
    class qtAttribute;

    class SMTKQTEXT_EXPORT qtAttributeRefItem : public qtItem
    {
      Q_OBJECT

    public:
      qtAttributeRefItem(smtk::attribute::ItemPtr,
        QWidget* parent,  qtBaseView* view, Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      virtual ~qtAttributeRefItem();

      virtual void setLabelVisible(bool);
      // this will turn on/off the Edit button.
      // Also, if the turning off, the Attribute widget will be turned off too
      virtual void setAttributeEditorVisible(bool);
      // turn on/off the attribute widget and the Collapse button.
      virtual void setAttributeWidgetVisible(bool);

    public slots:
      void onInputValueChanged();
      void onToggleAttributeWidgetVisibility();
      void onLaunchAttributeView();

    protected slots:
      virtual void updateItemData();
      virtual void setOutputOptional(int);

    protected:
      virtual void createWidget();
      virtual void refreshUI(QComboBox* combo);
      virtual void updateAttWidgetState(qtAttribute* qa);
      virtual void setAttributesVisible(bool visible);

    private:

      qtAttributeRefItemInternals *Internals;

    }; // class

    //A sublcass of QComboBox to refresh the list on popup
    class SMTKQTEXT_EXPORT qtAttRefCombo : public QComboBox
      {
      Q_OBJECT
      public:
        qtAttRefCombo(smtk::attribute::ItemPtr, QWidget * parent);
        virtual void showPopup();
        //virtual QSize sizeHint() const;
      private:
        smtk::attribute::WeakItemPtr m_RefItem;
      };

  }; // namespace attribute
}; // namespace smtk


#endif
