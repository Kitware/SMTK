//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtExtractContoursView - UI components for polygon contour operation View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef qtExtractContoursView_h
#define qtExtractContoursView_h

#include "smtk/extension/qt/qtBaseAttributeView.h"

class qtExtractContoursViewInternals;
class pqPipelineSource;

class qtExtractContoursView : public smtk::extension::qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtExtractContoursView);

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  qtExtractContoursView(const smtk::extension::ViewInfo& info);
  virtual ~qtExtractContoursView();

public slots:
  void updateUI() override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override;

  // This will be triggered by selecting different type
  // of edge operations, create-edge, edit-edge, or split-edge.
  virtual void operationSelected(const smtk::operation::OperationPtr& op);

protected slots:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void cancelOperation(const smtk::operation::OperationPtr&);
  virtual void startContourOperation();
  virtual void acceptContours(pqPipelineSource* contourSource);

protected:
  void createWidget() override;

private:
  qtExtractContoursViewInternals* Internals;

}; // class

#endif
