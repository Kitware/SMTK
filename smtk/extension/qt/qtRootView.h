//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtRootView - UI components for the attribute Root View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtRootView_h
#define __smtk_attribute_qtRootView_h

#include "smtk/extension/qt/qtBaseView.h"
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
      qtRootView(smtk::view::RootPtr, QWidget* p, qtUIManager* uiman);
      virtual ~qtRootView();
      void getChildView(smtk::view::Base::Type viewType,
        QList<qtBaseView*>& views);
      qtGroupView* getRootGroup();
      int advanceLevel();
      bool categoryEnabled();
      std::string currentCategory();

    public slots:
      virtual void showAdvanceLevel(int level);
      virtual void updateViewUI(int currentTab);
      virtual void enableShowBy(int enable);
      virtual void onShowCategory();
      virtual void showAdvanceLevelOverlay(bool);

    protected slots:
      virtual void onAdvanceLevelChanged(int levelIdx);

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
