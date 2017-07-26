//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAssignColorsView - UI component for extract terrain out of point clouds
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkTerrainExtractionView_h
#define smtkTerrainExtractionView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/vxl/operators/Exports.h"
#include "smtk/extension/vxl/operators/ui_smtkTerrainExtractionParameters.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

#include <QtCore/QObject>

class smtkTerrainExtractionViewInternals;
class pqTerrainExtractionManager;
class QString;
class QWidget;

class SMTKVXLOPERATORVIEWSEXT_EXPORT smtkTerrainExtractionView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkTerrainExtractionView(const smtk::extension::ViewInfo& info);
  virtual ~smtkTerrainExtractionView();
  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  Ui::TerrainExtractionParameters* terrainExtractionParameterUI();

  // virtual bool displayItem(smtk::attribute::ItemPtr);

public slots:
  virtual void updateUI() {} // NB: Subclass implementation causes crashes.
  virtual void requestModelEntityAssociation();
  virtual void onShowCategory() { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  virtual void valueChanged(smtk::attribute::ItemPtr optype);
  void onResolutionEditChanged(QString scaleString);

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);

  // Auto save slots
  bool onAutoSaveExtractFileName(); //Returns true if user selected a file
  // Mask size
  void onMaskSizeTextChanged(QString text);
  // Cache directory slots
  bool onSelectCacheDirectory(); //returns true if the user selected a directory

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();

  virtual void updateAttributeData();
  void onNumPointsCalculationFinshed(long numPoints);

protected:
  virtual void createWidget();
  virtual void setInfoToBeDisplayed() override;

private:
  smtkTerrainExtractionViewInternals* Internals;
  pqTerrainExtractionManager* TerrainExtractionManager;
};

#endif // smtkTerrainExtractionView_h
