//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtSimpleExpressionEvaluationView - UI components for the attribute Expression View
// with function evaluation
// .SECTION Description
// .SECTION See Also
// qtSimpleExpressionView

#ifndef __smtk_extension_qtSimpleExpressionEvaluationView_h
#define __smtk_extension_qtSimpleExpressionEvaluationView_h

#include "smtk/extension/paraview/widgets/Exports.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"

class SMTKPQWIDGETSEXT_EXPORT qtSimpleExpressionEvaluationView
  : public smtk::extension::qtSimpleExpressionView
{
  Q_OBJECT
public:
  static qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);
  qtSimpleExpressionEvaluationView(const smtk::extension::ViewInfo& info);
  virtual ~qtSimpleExpressionEvaluationView();

public slots:
  virtual void createFunctionWithExpression() override;

protected:
  void createWidget() override;

}; // class

#endif
