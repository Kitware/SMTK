//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperatorView - UI components for attribute Operator View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtOperatorView_h
#define __smtk_extension_qtOperatorView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

class qtOperatorViewInternals;
class QScrollArea;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT OperatorViewInfo : public ViewInfo
    {
    public:
    OperatorViewInfo(smtk::common::ViewPtr view, smtk::model::OperatorPtr targetOperator,
		     QWidget* parent, qtUIManager* uiman):
      ViewInfo(view, parent, uiman), m_operator(targetOperator) {}

      OperatorViewInfo(smtk::common::ViewPtr view, smtk::model::OperatorPtr targetOperator,
		       QWidget* parent, qtUIManager* uiman,
		       const std::map<std::string, QLayout *> &layoutDict):
      ViewInfo(view, parent, uiman, layoutDict), m_operator(targetOperator) {}

      OperatorViewInfo() {}
      smtk::model::OperatorPtr m_operator;
    };

    class SMTKQTEXT_EXPORT qtOperatorView : public qtBaseView
    {
      Q_OBJECT

    public:
      static qtBaseView *createViewWidget(const ViewInfo &info);

      qtOperatorView(const OperatorViewInfo &info);
      virtual ~qtOperatorView();

    public slots:
      virtual void showAdvanceLevelOverlay(bool show);
      virtual void requestModelEntityAssociation();
      virtual void onShowCategory()
       { this->updateAttributeData(); }
      virtual void onModifiedParameters();
      void onOperate();

    signals:
      void operationRequested(const smtk::model::OperatorPtr& brOp);

    protected:
      virtual void createWidget( );
      bool m_applied; // indicates if the current settings have been applied

    private:

      qtOperatorViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
