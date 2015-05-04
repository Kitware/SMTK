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
#include "smtk/common/View.h"

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
      static qtBaseView *createViewWidget(smtk::common::ViewPtr view, QWidget* p,
                                          qtUIManager* uiman);
      
      qtRootView(smtk::common::ViewPtr v, QWidget* p, qtUIManager* uiman);
      virtual ~qtRootView();
      void getChildView(const std::string &viewType,
                        QList<qtBaseView*>& views);
      qtGroupView* getRootGroup();
      virtual int advanceLevel();
      virtual bool categoryEnabled();
      virtual std::string currentCategory();

    public slots:
      virtual void showAdvanceLevel(int level);
      virtual void updateViewUI(int currentTab);
      virtual void enableShowBy(int enable);
      virtual void onShowCategory();
      virtual void showAdvanceLevelOverlay(bool);
      virtual void updateModelAssociation();

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
