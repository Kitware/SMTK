//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqTerrainExtractionManager_h
#define _pqTerrainExtractionManager_h

#include <smtk/SharedPtr.h>

#include "smtk/extension/vxl/widgets/Exports.h"
#include <QtCore/QObject>

class QWidget;
class pqOutputPort;
class pqDataRepresentation;
class pqPipelineSource;
class smtkTerrainExtractionView;

class SMTKVXLWIDGETSEXT_EXPORT pqTerrainExtractionManager : public QObject
{
  Q_OBJECT

public:
  pqTerrainExtractionManager(smtkTerrainExtractionView* view);
  ~pqTerrainExtractionManager() override;

  pqPipelineSource* getTerrainFilter() { return this->TerrainExtractFilter; }
  pqPipelineSource* getFullTerrainFilter() { return this->FullProcessTerrainExtractFilter; }

public slots:
  // when user has pick an aux_geom, update the view
  void onAuxGeomPicked();

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
  //  //resolution controls
  //  double ComputeResolution(pqPipelineSource* extractionFilter, bool computeDetailedScale);

  //  //methods called from onShow()
  //  void GuessCacheDirectory();
  //  void ComputeBasicResolution();

  //  pqDataRepresentation* createPreviewRepresentation(QString& filename);
  //  pqPipelineSource* setupFullProcessTerrainFilter();

  //  pqPipelineSource* PrepDataForTerrainExtraction();

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

  smtkTerrainExtractionView* View;
};

#endif /* __pqTerrainExtractionManager_h */
