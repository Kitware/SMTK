
//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqTerrainExtractionManager.h"

#include <QDir>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QVariant>
#include <QtCore/QFileInfo>

#include "vtkSmartPointer.h"
#include "vtkTransform.h"

#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMOutputPort.h"
#include "vtkSMSourceProxy.h"
#include <vtkSMPropertyHelper.h>

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Operation.h"

#include "vtkDataObject.h"

pqTerrainExtractionManager::pqTerrainExtractionManager()
{
  this->clear();
}

pqTerrainExtractionManager::~pqTerrainExtractionManager()
{
  if (this->TerrainExtractFilter)
  {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(this->TerrainExtractFilter);
  }
  if (this->FullProcessTerrainExtractFilter)
  {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->FullProcessTerrainExtractFilter);
  }
}

void pqTerrainExtractionManager::clear()
{
  this->TerrainExtractFilter = 0;
  this->FullProcessTerrainExtractFilter = 0;

  this->DetailedScale = 0;
  this->InputDims[0] = 1;
  this->InputDims[1] = 1;

  this->SaveRefineResults = false;
  this->CacheRefineDataForFullProcess = true;
}

void pqTerrainExtractionManager::setAuxGeom(smtk::model::AuxiliaryGeometry aux)
{
  this->clear();
  this->Aux_geom = aux;
  this->ComputeBasicResolution();
}

void pqTerrainExtractionManager::onResolutionScaleChange(QString scaleString)
{
  double scale = scaleString.toDouble();
  if (scale > 0)
  {
    long numPoints = (ceil(this->InputDims[0] / scale) * ceil(this->InputDims[1] / scale));
    emit this->numPointsCalculationFinshed(numPoints);
  }
}

void pqTerrainExtractionManager::ComputeBasicResolution()
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  // cleanup
  if (this->TerrainExtractFilter)
  {
    builder->destroy(this->TerrainExtractFilter);
    this->TerrainExtractFilter = 0;
  }

  pqPipelineSource* pdSource = this->PrepDataForTerrainExtraction();
  if (!pdSource)
  {
    smtk::io::Logger logger;
    smtkWarningMacro(logger, "Computing basic resolution failed due to empty pqPipelineSource.");
  }
  this->TerrainExtractFilter = builder->createFilter("filters", "TerrainExtract", pdSource);

  double computedScale = this->ComputeResolution(this->TerrainExtractFilter, false);

  // UpdatePropertyInformation in ComputeResolution
  this->DataTransform = pqSMAdaptor::getMultipleElementProperty(
    this->TerrainExtractFilter->getProxy()->GetProperty("GetDataTransform"));

  // release the memory
  builder->destroy(this->TerrainExtractFilter);
  builder->destroy(pdSource);
  this->TerrainExtractFilter = 0;

  QString scaleString;
  scaleString.setNum(computedScale);
  // Update resolutionEdit
  emit resolutionEditChanged(scaleString);
}

void pqTerrainExtractionManager::ComputeDetailedResolution()
{
  if (this->DetailedScale == 0)
  {
    // FIXME: pqCMBLIDARTerrainExtractionManager::setupFullProcessTerrainFilter
    // has a complex logic to get pqPieLineSource. For now it just simply read
    // the data without updating pieces
    pqPipelineSource* appendedSource = this->setupFullProcessTerrainFilter();
    if (!appendedSource)
    {
      return;
    }

    QMessageBox msgBox;
    msgBox.setText("Computing spacing (may take awhile)...");
    msgBox.setModal(false);
    msgBox.show();

    if (this->FullProcessTerrainExtractFilter)
    {

      //users first time clicking on the detailed
      //res button. Store the computed value so we can save time on subsequent clicks
      this->DetailedScale = this->ComputeResolution(this->FullProcessTerrainExtractFilter, true);

      // after "setting up refine", can get rid of appendedSource
      pqSMAdaptor::setInputProperty(
        this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("Input"), 0, 0);
      this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();
      pqApplicationCore::instance()->getObjectBuilder()->destroy(appendedSource);
    }
  }
  //if you click on the button after the first time, we reset the spinbox to the computed estimate
  QString scaleString;
  scaleString.setNum(this->DetailedScale);
  emit resolutionEditChanged(scaleString);
}

double pqTerrainExtractionManager::ComputeResolution(
  pqPipelineSource* extractionFilter, bool computeDetailedScale)
{
  pqSMAdaptor::setElementProperty(
    extractionFilter->getProxy()->GetProperty("ExecuteMode"), 0); // setup refine

  //set if we want a detailed scan of the dataset to determin the scaling
  pqSMAdaptor::setElementProperty(
    extractionFilter->getProxy()->GetProperty("SetComputeInitialScale"), computeDetailedScale);

  pqSMAdaptor::setElementProperty(
    extractionFilter->getProxy()->GetProperty("ComputeDataTransform"), !computeDetailedScale);

  extractionFilter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(extractionFilter->getProxy())->UpdatePipeline();
  extractionFilter->getProxy()->UpdatePropertyInformation();

  QList<QVariant> vbounds = pqSMAdaptor::getMultipleElementProperty(
    extractionFilter->getProxy()->GetProperty("InputBounds"));

  // query the server for the computed scale
  double scale =
    pqSMAdaptor::getElementProperty(extractionFilter->getProxy()->GetProperty("GetInitialScale"))
      .toDouble();

  // Update the render view
  pqApplicationCore::instance()->render();

  this->InputDims[0] = (vbounds[1].toDouble() - vbounds[0].toDouble());
  this->InputDims[1] = (vbounds[3].toDouble() - vbounds[2].toDouble());

  return scale;
}

pqPipelineSource* pqTerrainExtractionManager::PrepDataForTerrainExtraction()
{
  // Use the same logic in pqCMBLIDARTerrainExtractionManager::createPreviewRepresentation
  QString filename(QString::fromStdString(this->Aux_geom.url()));
  QFileInfo fileInfo(filename);
  QString extension(fileInfo.completeSuffix().toLower());
  QStringList filenameSL;
  filenameSL << filename;
  pqServer* server = pqApplicationCore::instance()->getActiveServer();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  builder->blockSignals(true);
  pqPipelineSource* readerSource;
  double minPt[3] = { 0, 0, 0 }, maxPt[3] = { 0, 0, 0 };
  if (extension == "pts" || extension == "xyz")
  {
    readerSource = builder->createReader("sources", "LIDARReader", filenameSL, server);
    if (!readerSource)
    {
      std::cerr << "No LIDARReader source when preparing data for extraction" << std::endl;
      return nullptr;
    }

    vtkSMPropertyHelper(readerSource->getProxy(), "MaxNumberOfPoints").Set(100000);
    vtkSMPropertyHelper(readerSource->getProxy(), "LimitToMaxNumberOfPoints").Set(1);
    readerSource->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(readerSource->getProxy())->UpdatePipeline();
    readerSource->getProxy()->UpdatePropertyInformation();
    double* dataBounds =
      vtkSMDoubleVectorProperty::SafeDownCast(readerSource->getProxy()->GetProperty("DataBounds"))
        ->GetElements();
    minPt[2] = dataBounds[4];
    maxPt[2] = dataBounds[5];
  }
  else
  {
    readerSource = builder->createReader(
      "sources", "CMBGeometryReader", filenameSL, pqApplicationCore::instance()->getActiveServer());
    pqPipelineSource* polyDataStatsFilter =
      builder->createFilter("filters", "PolyDataStatsFilter", readerSource);
    vtkSMSourceProxy::SafeDownCast(polyDataStatsFilter->getProxy())->UpdatePipeline();
    minPt[2] = pqSMAdaptor::getMultipleElementProperty(
                 polyDataStatsFilter->getProxy()->GetProperty("GeometryBounds"), 4)
                 .toDouble();
    maxPt[2] = pqSMAdaptor::getMultipleElementProperty(
                 polyDataStatsFilter->getProxy()->GetProperty("GeometryBounds"), 5)
                 .toDouble();
    builder->destroy(polyDataStatsFilter);
  }
  builder->blockSignals(false);

  pqPipelineSource* elevationSource =
    builder->createFilter("filters", "LIDARElevationFilter", readerSource);
  if (!elevationSource)
  {
    std::cerr << "No LIDARElevationFilter filter when preparing data for extraction" << std::endl;
    return nullptr;
  }

  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("LowPoint"), 0, minPt[0]);
  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("LowPoint"), 1, minPt[1]);
  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("LowPoint"), 2, minPt[2]);
  elevationSource->getProxy()->UpdateProperty("LowPoint", true);
  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("HighPoint"), 0, maxPt[0]);
  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("HighPoint"), 1, maxPt[1]);
  pqSMAdaptor::setMultipleElementProperty(
    elevationSource->getProxy()->GetProperty("HighPoint"), 2, maxPt[2]);
  elevationSource->getProxy()->UpdateProperty("HighPoint", true);

  vtkSMSourceProxy::SafeDownCast(elevationSource->getProxy())->UpdatePipeline();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(elevationSource->getProxy()));
  pdSource->updatePipeline();

  builder->destroy(elevationSource);
  builder->destroy(readerSource);

  return pdSource;
}

pqPipelineSource* pqTerrainExtractionManager::setupFullProcessTerrainFilter()
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  // cleanup
  if (this->FullProcessTerrainExtractFilter)
  {
    builder->destroy(this->FullProcessTerrainExtractFilter);
    this->FullProcessTerrainExtractFilter = 0;
  }

  // Set value for FullProcessTerrainExtractFilter and get appendedSource
  pqPipelineSource* appendedSource = this->PrepDataForTerrainExtraction();
  if (!appendedSource)
  {
    smtk::io::Logger logger;
    smtkWarningMacro(logger, "Setting up full process terrain filter failed due to emtpy source");
  }

  this->FullProcessTerrainExtractFilter =
    builder->createFilter("filters", "TerrainExtract", appendedSource);

  // setup the data transform
  pqSMAdaptor::setMultipleElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetDataTransform"),
    this->DataTransform);
  this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();

  return appendedSource;
}

void pqTerrainExtractionManager::onProcesssFullData(double scale, double maskSize,
  QFileInfo /*cacheFileInfo*/, QFileInfo autoSaveInfo, bool computeColor, bool viewOutput,
  bool pickCustomResult)
{
  // TODO: Hide all aux_geoms?
  QMessageBox warningBox;
  warningBox.setText("Extracting terrain (may take a while)...");
  warningBox.exec();

  // for now, assume one shot... do not save refine output (handled upon exit by
  // destroying the filter)
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  // what levels to process / extract
  int minLevel = 0;
  int maxLevel = 1;

  ////////// Refine Setup Phase (if necessary) ////////////
  if (!this->FullProcessTerrainExtractFilter) // haven't done refine yet on full data
  {
    pqPipelineSource* appendedSource = this->setupFullProcessTerrainFilter();
    if (!appendedSource)
    {
      smtk::io::Logger logger;
      smtkWarningMacro(logger, "Process full data failed due to empty pqPipelineSource");
      return;
    }

    //set the mask size to use
    pqSMAdaptor::setElementProperty(
      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetMaskSize"), maskSize);

    pqSMAdaptor::setElementProperty(
      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"),
      0); // setup refine

    this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
      ->UpdatePipeline();

    // after "setting up refine", can get rid of appendedSource
    pqSMAdaptor::setInputProperty(
      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("Input"), 0, 0);
    this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();
    builder->destroy(appendedSource);
  }

  ////////// Setup of Refine and Extraction phases ////////////

  // Set the mask size to use
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetMaskSize"), maskSize);

  // TODO: Implement cache logic for elementProperty of FullProcessTerrainExtractFilter as
  // CacheRefineResultsToDisk and IntermediateResultsPath

  if (this->SaveRefineResults)
  {
    // write visualization to disk!
    pqSMAdaptor::setElementProperty(this->FullProcessTerrainExtractFilter->getProxy()->GetProperty(
                                      "RefineVisualizationOutputMode"),
      0);
  }
  else
  {
    // Now, do NOT want to produce polydata ouptut of each refine level, because we don't display it!
    pqSMAdaptor::setElementProperty(this->FullProcessTerrainExtractFilter->getProxy()->GetProperty(
                                      "RefineVisualizationOutputMode"),
      2);
  }

  // setup for any file to be written to disk

  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPath"),
    autoSaveInfo.absolutePath().toStdString().c_str());

  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputBaseFileName"),
    autoSaveInfo.baseName().toStdString().c_str());

  QString extension(autoSaveInfo.completeSuffix().toLower());
  if ((extension == "pts") || (extension == "xyz"))
  {
    pqSMAdaptor::setElementProperty(
      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPtsFormat"), 1);
  }
  else
  {
    pqSMAdaptor::setElementProperty(
      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPtsFormat"), 0);
  }

  ////////// Refine Phase ////////////

  //set the scaling to use
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetInitialScale"), scale);

  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"), 1); // refine

  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"), 0);

  this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();

  vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
    ->UpdatePipeline();

  int aborted = pqSMAdaptor::getElementProperty(
                  this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"))
                  .toInt();
  if (aborted)
  {
    builder->destroy(this->FullProcessTerrainExtractFilter);
    this->FullProcessTerrainExtractFilter = 0;
    return;
  }

  // go ahead and update max refine level for the full refine
  this->FullProcessTerrainExtractFilter->getProxy()->UpdatePropertyInformation();
  maxLevel = pqSMAdaptor::getElementProperty(
               this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("NumberOfLevels"))
               .toInt() -
    1;

  ////////// Extraction Phase ////////////

  // Now, the extraction
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("WriteExtractionResultsToDisk"),
    true);

  // Pass color and/or info from input to output (if present)?
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("DetermineIntensityAndColor"),
    computeColor);

  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"), 2); // extract
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("MinExtractLevel"), minLevel);
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("MaxExtractLevel"), maxLevel);
  pqSMAdaptor::setElementProperty(
    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"), 0);
  this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();

  vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
    ->UpdatePipeline();

  aborted = pqSMAdaptor::getElementProperty(
              this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"))
              .toInt();

  QMessageBox msgBox;
  if (aborted)
  {
    msgBox.setText("Terrain extraction Aborted");
  }
  else
  {
    msgBox.setText("The Terrain extraction has completed");
  }
  msgBox.exec();

  if (viewOutput)
  {
    this->ViewResults(autoSaveInfo, pickCustomResult);
  }

  // right now not supporting repeat full process
  builder->destroy(this->FullProcessTerrainExtractFilter);
  this->FullProcessTerrainExtractFilter = 0;

  // Update the render view
  pqApplicationCore::instance()->render();
}

void pqTerrainExtractionManager::onSaveRefineResultsChange(bool change)
{
  this->SaveRefineResults = change;
}

void pqTerrainExtractionManager::ViewResults(QFileInfo autoSaveInfo, bool pickCustomResult)
{
  smtk::attribute::AttributePtr att = this->AddAux_GeomOp.lock()->parameters();
  std::string filename;
  if (pickCustomResult)
  { // Let user choose a file
    emit showPickResultFileDialog(filename);
  }
  else
  { // Load the finest result
    filename += autoSaveInfo.absolutePath().toStdString() + "/";
    filename += autoSaveInfo.baseName().toStdString();
    filename += "_00.";
    filename += autoSaveInfo.completeSuffix().toStdString();
  }
  att->associateEntity(qtActiveObjects::instance().activeModel());
  att->findFile("url")->setValue(filename);
  emit viewTerrainExtractionResults();
}
