//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtGroupView - UI components for Group View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtGroupView_h
#define __smtk_attribute_qtGroupView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/view/Base.h"

class qtGroupViewInternals;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtGroupView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtGroupView(smtk::view::BasePtr, QWidget* p, qtUIManager* uiman);
      virtual ~qtGroupView();

      void getChildView(smtk::view::Base::Type viewType,
        QList<qtBaseView*>& views);
      qtBaseView* getChildView(int pageIndex);

      virtual void addChildView(qtBaseView*);
      virtual void clearChildViews();
      QList<qtBaseView*>& childViews() const;

    public slots:
      virtual void updateUI();
      virtual void showAdvanceLevelOverlay(bool show);

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
