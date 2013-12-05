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
// .NAME qtRootView - a Root View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtRootView_h
#define __smtk_attribute_qtRootView_h

#include "smtk/Qt/qtBaseView.h"
#include "smtk/view/Base.h"

class qtRootViewInternals;
class QScrollArea;

namespace smtk
{
  namespace attribute
  {
    class qtGroupView;

    class QTSMTK_EXPORT qtRootView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtRootView(smtk::view::RootPtr, QWidget* p);
      virtual ~qtRootView();
      void getChildView(smtk::view::Base::Type viewType,
        QList<qtBaseView*>& views);
      qtGroupView* getRootGroup();
      bool showAdvanced();
      bool categoryEnabled();
      std::string currentCategory();

    public slots:
      virtual void onShowAdvanced(int show);
      virtual void updateViewUI(int currentTab);
      virtual void enableShowBy(int enable);
      virtual void onShowCategory();

    protected:
      virtual void createWidget( );
      virtual void initRootTabGroup( );
      virtual void filterByCategory();

      QScrollArea *ScrollArea;

    private:

      qtRootViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
