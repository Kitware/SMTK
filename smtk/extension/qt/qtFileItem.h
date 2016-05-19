//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtFileItem - UI components for attribute FileItem
// .SECTION Description
//
// This is widget is designed to let applications connect a file
// browser widget for selecting files, by connecting to the
// launchFileBrowser signal and onInputValueChanged slot.
//
// The class can also instantiate its own file browser.
// To use this option, you can call the enableFileBrowser() method
// in response to the qtUIManager::fileItemCreated signal.
// In more typical applications, use the second, optional
// argument in qtUIManager.initializeUI() to enable the option.
//
// Note: The current code does not support multiple file
// or directory selection.
//
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtFileItem_h
#define __smtk_extension_qtFileItem_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"

class qtFileItemInternals;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtFileItem : public qtItem
    {
      Q_OBJECT

    public:
      qtFileItem(smtk::attribute::FileItemPtr, QWidget* parent, qtBaseView* bview,
                 Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      qtFileItem(smtk::attribute::DirectoryItemPtr, QWidget* parent, qtBaseView* bview,
                 Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      virtual ~qtFileItem();
      void enableFileBrowser(bool state=true);
      bool isDirectory();
      virtual void setLabelVisible(bool);
      virtual void setInputValue(const QString&);

    public slots:
      virtual void onInputValueChanged();
      virtual void onLaunchFileBrowser();
      virtual void setOutputOptional(int);
      virtual void updateFileComboList();

    signals:
      void launchFileBrowser();

    protected slots:
      virtual void updateItemData();

    protected:
      virtual void createWidget();
      QWidget* createFileBrowseWidget(int elementIdx);

    private:

      qtFileItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
