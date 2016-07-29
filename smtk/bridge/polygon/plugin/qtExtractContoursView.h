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
  static smtk::extension::qtBaseView *createViewWidget(
         const smtk::extension::ViewInfo &info);

  qtExtractContoursView(const smtk::extension::ViewInfo &info);
  virtual ~qtExtractContoursView();

public slots:
  virtual void showAdvanceLevelOverlay(bool show);
  virtual void requestModelEntityAssociation();
  virtual void onShowCategory()
   { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of edge operations, create-edge, edit-edge, or split-edge.
  virtual void operationSelected(const smtk::model::OperatorPtr& op);
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  virtual void valueChanged(smtk::attribute::ItemPtr optype);

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void startContourOperation();
  virtual void acceptContours(pqPipelineSource* contourSource);

protected:
  virtual void updateAttributeData();
  virtual void createWidget( );

private:

  qtExtractContoursViewInternals *Internals;

}; // class


#endif
