/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME qtFileItem - an Expression item
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

#ifndef __smtk_attribute_qtFileItem_h
#define __smtk_attribute_qtFileItem_h

#include "smtk/Qt/qtItem.h"

class qtFileItemInternals;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtFileItem : public qtItem
    {
      Q_OBJECT

    public:
      qtFileItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview, bool dirOnly=false);
      virtual ~qtFileItem();
      void enableFileBrowser(bool state=true);
      bool isDirectory();

    public slots:
      virtual void onInputValueChanged();
      virtual void onLaunchFileBrowser();

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
