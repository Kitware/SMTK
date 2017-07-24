//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqTerrainExtractionManager - helper class for smtkTerrainExtraction view to connect UI part with extraction logic. All UI changes are done
// in the view
// .SECTION Description
// TODO: Add progress bar and suport for more file types
// .SECTION See Also
// pqCMBLIDARTerrainExtractionManager

#ifndef _pqTerrainExtractionManager_h
#define _pqTerrainExtractionManager_h

#include <smtk/SharedPtr.h>

#include "smtk/extension/vxl/widgets/Exports.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include <QtCore/QObject>

class QString;
class QWidget;
class pqOutputPort;
class pqDataRepresentation;
class pqPipelineSource;

class SMTKVXLWIDGETSEXT_EXPORT pqTerrainExtractionManager : public QObject
{
  Q_OBJECT

public:
  pqTerrainExtractionManager();
  ~pqTerrainExtractionManager() override;

  //  pqPipelineSource* getTerrainFilter() { return this->TerrainExtractFilter; }
  //  pqPipelineSource* getFullTerrainFilter() { return this->FullProcessTerrainExtractFilter; }

signals:
  void numPointsCalculationFinshed(long numPoints);
  void resolutionEditChanged(QString scaleString);

public slots:
  // set aux_geom input then compute basic resolution and guess cache dir
  void setAuxGeom(smtk::model::AuxiliaryGeometry aux);

protected slots:
  //  void onProcesssFullData();

  //  //auto save slots
  //  bool onAutoSaveExtractFileName(); //returns true if the user selected a file

  //  //cache directory slots
  //  bool onSelectCacheDirectory(); //returns true if the user selected a directory

  //  //resolution controls
  void onResolutionScaleChange(QString scaleString);
  //void ComputeDetailedResolution();

  //  //mask size control
  //  void onMaskSizeTextChanged(QString);

  //  //if we are saving the refine results
  //  void onSaveRefineResultsChange(bool change);

protected:
  //resolution controls
  double ComputeResolution(pqPipelineSource* extractionFilter, bool computeDetailedScale);

  //methods called from setAuxGeom()
  void ComputeBasicResolution();

  //  pqDataRepresentation* createPreviewRepresentation(QString& filename);
  //  pqPipelineSource* setupFullProcessTerrainFilter();

  // Process the Aux_geom and convert it into a pqPipelineSource
  pqPipelineSource* PrepDataForTerrainExtraction();

  //  // load the auxgeom back in!
  ////  void addExtractionOutputToTree(
  ////    int minLevel, int maxLevel, double initialScale, QFileInfo& autoSaveInfo);

  // Description:
  // Some internal ivars.
  bool CacheRefineDataForFullProcess;
  bool SaveRefineResults;

  double DetailedScale;
  double InputDims[2];

  pqPipelineSource* TerrainExtractFilter;
  pqPipelineSource* FullProcessTerrainExtractFilter;

  QList<QVariant> DataTransform;

  std::vector<pqPipelineSource*> PDSources;

  smtk::model::AuxiliaryGeometry Aux_geom;
};

#endif /* __pqTerrainExtractionManager_h */
