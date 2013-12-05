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
// .NAME qtGroupView - a Group View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtGroupView_h
#define __smtk_attribute_qtGroupView_h

#include "smtk/Qt/qtBaseView.h"
#include "smtk/view/Base.h"

class qtGroupViewInternals;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtGroupView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtGroupView(smtk::view::BasePtr, QWidget* p);
      virtual ~qtGroupView();

      void getChildView(smtk::view::Base::Type viewType,
        QList<qtBaseView*>& views);
      qtBaseView* getChildView(int pageIndex);

      virtual void addChildView(qtBaseView*);
      virtual void clearChildViews();
      QList<qtBaseView*>& childViews() const;

    public slots:
      virtual void showAdvanced(int show);
      virtual void updateUI();

    protected:
      virtual void createWidget( );
      virtual void addTabEntry(qtBaseView*);
      virtual void addTileEntry(qtBaseView*);

    private:

      qtGroupViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
