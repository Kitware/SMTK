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

#include "smtk/extension/qt/qtBaseView.h"

class qtExtractContoursViewInternals;
class pqPipelineSource;

class qtExtractContoursView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  qtExtractContoursView(const smtk::extension::ViewInfo& info);
  virtual ~qtExtractContoursView();

public slots:
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of edge operations, create-edge, edit-edge, or split-edge.
  virtual void operationSelected(const smtk::model::OperatorPtr& op);

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void startContourOperation();
  virtual void acceptContours(pqPipelineSource* contourSource);

protected:
  void updateAttributeData() override;
  void createWidget() override;

private:
  qtExtractContoursViewInternals* Internals;

}; // class

#endif
