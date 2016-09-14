//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtInputsItem - UI components for attribute ValueItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtInputsItem_h
#define __smtk_extension_qtInputsItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"
#include <QDoubleValidator>

class qtInputsItemInternals;
class QBoxLayout;
class QLayout;

namespace smtk
{
  namespace extension
  {
    class qtUIManager;

    class SMTKQTEXT_EXPORT qtInputsItem : public qtItem
    {
      Q_OBJECT

    public:
      qtInputsItem(smtk::attribute::ItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtInputsItem();
      virtual void setLabelVisible(bool);

    public slots:
      void setOutputOptional(int);
      void onExpressionReferenceChanged();
      void onTextEditChanged();
      void onLineEditChanged();
      void onLineEditFinished();
      void onInputValueChanged(QObject*);

    protected slots:
      virtual void updateItemData();
      virtual void onAddNewValue();
      virtual void onRemoveValue();
      void displayExpressionWidget(bool checkstate);

    protected:
      virtual void createWidget();
      virtual void loadInputValues();
      virtual void updateUI();
      virtual void addInputEditor(int i);
      virtual void updateExtensibleState();
      virtual void clearChildWidgets();
      virtual QWidget* createInputWidget(smtk::attribute::ItemPtr,
					 int elementIdx, QLayout* childLayout);
      virtual QWidget* createEditBox(smtk::attribute::ItemPtr,
				     int elementIdx, QWidget* pWidget);
      virtual QWidget* createExpressionRefWidget(smtk::attribute::ItemPtr,
						 int elementIdx);


    private:

      qtInputsItemInternals *Internals;

    }; // class

    //A sublcass of QDoubleValidator to fixup input outside of range
    class SMTKQTEXT_EXPORT qtDoubleValidator : public QDoubleValidator
    {
      Q_OBJECT
    public:
        qtDoubleValidator(QObject * parent);
        virtual void fixup(QString &input) const;

        void setUIManager(qtUIManager* uiman);
    private:
      qtUIManager* UIManager;
    };

    //A sublcass of QIntValidator to fixup input outside of range
    class SMTKQTEXT_EXPORT qtIntValidator : public QIntValidator
      {
      Q_OBJECT
      public:
        qtIntValidator(QObject * parent);
        virtual void fixup(QString &input) const;

        void setUIManager(qtUIManager* uiman);

      private:
        qtUIManager* UIManager;
      };

  }; // namespace extension
}; // namespace smtk

#endif
