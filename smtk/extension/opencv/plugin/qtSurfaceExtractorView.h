//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtSurfaceExtractorView - UI components for polygon contour operation View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef qtSurfaceExtractorView_h
#define qtSurfaceExtractorView_h

#include "smtk/extension/qt/qtBaseView.h"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class qtSurfaceExtractorViewInternals;

class qtSurfaceExtractorView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  qtSurfaceExtractorView(const smtk::extension::ViewInfo& info);
  virtual ~qtSurfaceExtractorView();

public slots:
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of edge operations, create-edge, edit-edge, or split-edge.
  virtual void operationSelected(const smtk::operation::NewOpPtr& op);

protected slots:
  virtual void requestOperation(const smtk::operation::NewOpPtr& op);
  virtual void cancelOperation(const smtk::operation::NewOpPtr&);
  virtual void startContourOperation();
  virtual void acceptContours(vtkSmartPointer<vtkPolyData>);

protected:
  void updateAttributeData() override;
  void createWidget() override;

private:
  qtSurfaceExtractorViewInternals* Internals;

}; // class

#endif
