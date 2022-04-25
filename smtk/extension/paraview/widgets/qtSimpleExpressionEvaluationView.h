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

#ifndef smtk_extension_qtSimpleExpressionEvaluationView_h
#define smtk_extension_qtSimpleExpressionEvaluationView_h

#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"

class SMTKPQWIDGETSEXT_EXPORT qtSimpleExpressionEvaluationView
  : public smtk::extension::qtSimpleExpressionView
{
  Q_OBJECT
public:
  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtSimpleExpressionEvaluationView(const smtk::view::Information& info);
  ~qtSimpleExpressionEvaluationView() override;

public Q_SLOTS:
  void createFunctionWithExpression() override;

protected:
  void createWidget() override;

}; // class

#endif
