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
// smtkTerrainExtractionView owns pqTerrainExtractionMananger to avoid cyclic dendency

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
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  void valueChanged(smtk::attribute::ItemPtr optype) override;
  void onResolutionEditChanged(QString scaleString);

protected slots:
  virtual void requestOperation(const smtk::operation::NewOpPtr& op);
  virtual void cancelOperation(const smtk::operation::NewOpPtr&);

  // Auto save slots
  bool onAutoSaveExtractFileName(); //Returns true if user selected a file
  // Mask size
  void onMaskSizeTextChanged(QString text);
  // Cache directory slots
  bool onSelectCacheDirectory(); //returns true if the user selected a directory

  // A relay function to pass needed data to TerrainExtractionManager
  void onProcessFullExtraction();
  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();

  void onViewTerrainExtractionResults();

  void onShowPickResultFileDialog(std::string& filename);

  void updateAttributeData() override;
  void onNumPointsCalculationFinshed(long numPoints);

protected:
  void createWidget() override;
  virtual void setInfoToBeDisplayed() override;

private:
  smtkTerrainExtractionViewInternals* Internals;
  pqTerrainExtractionManager* TerrainExtractionManager;
};

#endif // smtkTerrainExtractionView_h
