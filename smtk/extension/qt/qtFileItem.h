//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtFileItem - UI components for attribute ValueItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtFileItem_h
#define __smtk_extension_qtFileItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

class qtFileItemInternals;
class QBoxLayout;
class QWidget;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtFileItem : public qtItem
    {
      Q_OBJECT

    public:
      qtFileItem(smtk::attribute::FileSystemItemPtr, QWidget* p,
        qtBaseView* bview, Qt::Orientation enumOrient = Qt::Horizontal);
      virtual ~qtFileItem();
      virtual void setLabelVisible(bool);

      void enableFileBrowser(bool state=true);
      bool isDirectory();
      virtual void setInputValue(const QString&);

    public slots:
      virtual void onInputValueChanged();
      void setOutputOptional(int);
      virtual void onLaunchFileBrowser();
      virtual void updateFileComboList(const QString&);

    signals:
      void launchFileBrowser();

    protected slots:
      virtual void updateItemData();
      virtual void onAddNewValue();
      virtual void onRemoveValue();
      virtual void setActiveField(QWidget*);

    protected:
      virtual void createWidget();
      QWidget* createFileBrowseWidget(int elementIdx);
      virtual void loadInputValues();
      virtual void updateUI();
      virtual void addInputEditor(int i);
      virtual void updateExtensibleState();
      virtual void clearChildWidgets();

    private:

      qtFileItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
