//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDateTimeItem - UI components for attribute DateTimeItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtDateTimeItem_h
#define __smtk_extension_qtDateTimeItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"

class qtDateTimeItemInternals;
class QDateTime;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtDateTimeItem : public qtItem
    {
      Q_OBJECT

    public:
      qtDateTimeItem(smtk::attribute::DateTimeItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Vertical);
      virtual ~qtDateTimeItem();
      virtual void setLabelVisible(bool);

    public slots:
      void setOutputOptional(int);

    signals:

    protected slots:
      virtual void updateItemData();
      virtual void onDateTimeChanged(const QDateTime&);
      //virtual void onAdvanceLevelChanged(int levelIdx);
      virtual void onChildWidgetSizeChanged();
      //virtual void onAddNewValue();
      //virtual void onRemoveValue();
      void onTimeZoneComboChanged(int index);

    protected:
      virtual void createWidget();
      QWidget *createDateTimeWidget(int elementIdx);
      virtual void loadInputValues();
      virtual void updateUI();
      virtual void addInputEditor(int i);
      virtual void updateExtensibleState();
      virtual void clearChildWidgets();

    private:
      qtDateTimeItemInternals *Internals;
    }; // class qDateTimeItem
  }; // namespace attribute
}; // namespace smtk

#endif
