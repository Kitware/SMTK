
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
#include <QFileInfo>
#include <QIcon>
#include <QList>
#include <QMessageBox>
#include <QPixmap>
#include <QVariant>

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
#include "smtk/extension/paraview/widgets/qtArcWidget.h"
#include "smtk/io/Logger.h"

#include "vtkDataObject.h"

pqTerrainExtractionManager::pqTerrainExtractionManager()
{
  //this->View = view;

  this->TerrainExtractFilter = 0;
  this->FullProcessTerrainExtractFilter = 0;

  this->DetailedScale = 0;
  this->InputDims[0] = 1;
  this->InputDims[1] = 1;

  this->SaveRefineResults = false;
  this->CacheRefineDataForFullProcess = true;
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

void pqTerrainExtractionManager::setAuxGeom(smtk::model::AuxiliaryGeometry aux)
{
  this->Aux_geom = aux;
  std::cout << "set AuxGeom: " << this->Aux_geom.url() << std::endl;
  this->ComputeBasicResolution();
  this->CacheRefineDataForFullProcess = true;
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
  /*******************************************************************/
  std::cout << "ComputeResolution with dim as " << this->InputDims[0] << " " << this->InputDims[1]
            << " with computeDetailedScale as " << computeDetailedScale << std::endl;
  /*******************************************************************/

  return scale;
}

//void pqTerrainExtractionManager::onMaskSizeTextChanged(QString text)
//{
//  if (text.size() == 0 || text == ".")
//  {
//    //we want to exempt empty strings from the below changes so that people
//    //can delete the current text.
//    //we want to also exempt a string starting with the decimal dot.
//    return;
//  }
//  QLineEdit* masksize = this->LIDARPanel->getGUIPanel()->MaskSize;
//  const QDoubleValidator* validator = qobject_cast<const QDoubleValidator*>(masksize->validator());
//  if (validator)
//  {
//    int pos = 0; //needed just as paramter for the double validator
//    QValidator::State state = validator->validate(text, pos);
//    if (state != QValidator::Acceptable)
//    {
//      //convert this to the closest value
//      double value = text.toDouble();
//      value = (value < validator->bottom()) ? validator->bottom() : validator->top();
//      masksize->setText(QString::number(value));
//    }
//  }
//}

pqPipelineSource* pqTerrainExtractionManager::PrepDataForTerrainExtraction()
{
  // Use the same logic in pqCMBLIDARTerrainExtractionManager::createPreviewRepresentation
  /***********************************************************************/
  std::cout << "pqterrainExtractionManager::PrepDataForTerrainExtraction url: "
            << this->Aux_geom.url() << std::endl;
  /***********************************************************************/
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

//void pqTerrainExtractionManager::onProcesssFullData()
//{
//  // turn off all representations
//  QList<pqCMBLIDARPieceObject*> pieces =
//    this->LIDARCore->getLIDARPieceTable()->getVisiblePieceObjects();
//  for (int i = 0; i < pieces.count(); i++)
//  {
//    pieces[i]->setVisibility(0);
//  }

//  // for now, assume one shot... do not save refine output (handled upon exit by
//  // destroying the filter)
//  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

//  // what levels to process / extract
//  int minLevel = 0;
//  int maxLevel = 1;
//  double scale = this->LIDARPanel->getGUIPanel()->resolutionEdit->text().toDouble();
//  double maskSize = this->LIDARPanel->getGUIPanel()->MaskSize->text().toDouble();

//  this->LIDARPanel->getGUIPanel()->tabWidget->setEnabled(false);
//  emit this->enableMenuItems(false);

//  ////////// Refine Setup Phase (if necessary) ////////////
//  if (!this->FullProcessTerrainExtractFilter) // haven't done refine yet on full data
//  {
//    pqPipelineSource* appendedSource = this->setupFullProcessTerrainFilter();
//    if (!appendedSource)
//    {
//      this->LIDARPanel->getGUIPanel()->tabWidget->setEnabled(true);
//      emit this->enableMenuItems(true);
//      return;
//    }

//    this->LIDARCore->updateProgress(QString("Refining terrain"), 0);

//    //set the mask size to use
//    pqSMAdaptor::setElementProperty(
//      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetMaskSize"), maskSize);

//    pqSMAdaptor::setElementProperty(
//      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"),
//      0); // setup refine

//    this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();
//    vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
//      ->UpdatePipeline();

//    // after "setting up refine", can get rid of appendedSource
//    pqSMAdaptor::setInputProperty(
//      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("Input"), 0, 0);
//    this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();
//    builder->destroy(appendedSource);
//  }

//  ////////// Setup of Refine and Extraction phases ////////////

//  QFileInfo cacheFileInfo(this->LIDARPanel->getGUIPanel()->CacheDirectoryLabel->text());

//  //set the mask size to use
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetMaskSize"), maskSize);

//  // cache refine results to disk
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("CacheRefineResultsToDisk"), 1);

//  //we ALWAYS store cache driectory as the refineResultsInfo
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("IntermediateResultsPath"),
//    cacheFileInfo.absoluteFilePath().toStdString().c_str());

//  if (this->SaveRefineResults)
//  {
//    // write visualization to disk!
//    pqSMAdaptor::setElementProperty(this->FullProcessTerrainExtractFilter->getProxy()->GetProperty(
//                                      "RefineVisualizationOutputMode"),
//      0);
//  }
//  else
//  {
//    // Now, do NOT want to produce polydata ouptut of each refine level, because we don't display it!
//    pqSMAdaptor::setElementProperty(this->FullProcessTerrainExtractFilter->getProxy()->GetProperty(
//                                      "RefineVisualizationOutputMode"),
//      2);
//  }

//  // setup for any file to be written to disk
//  QFileInfo autoSaveInfo(this->LIDARPanel->getGUIPanel()->autoSaveLabel->text());

//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPath"),
//    autoSaveInfo.absolutePath().toStdString().c_str());

//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputBaseFileName"),
//    autoSaveInfo.baseName().toStdString().c_str());

//  QString extension(autoSaveInfo.completeSuffix().toLower()), writerName;
//  if ((extension == "pts") || (extension == "xyz"))
//  {
//    pqSMAdaptor::setElementProperty(
//      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPtsFormat"), 1);
//  }
//  else
//  {
//    pqSMAdaptor::setElementProperty(
//      this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("OutputPtsFormat"), 0);
//  }

//  ////////// Refine Phase ////////////

//  //set the scaling to use
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("SetInitialScale"), scale);

//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"), 1); // refine

//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"), 0);

//  this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();

//  this->LIDARCore->enableAbort(true);
//  vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
//    ->UpdatePipeline();
//  this->LIDARCore->enableAbort(false);

//  int aborted = pqSMAdaptor::getElementProperty(
//                  this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"))
//                  .toInt();
//  if (aborted)
//  {
//    this->LIDARPanel->getGUIPanel()->tabWidget->setEnabled(true);
//    emit this->enableMenuItems(true);
//    builder->destroy(this->FullProcessTerrainExtractFilter);
//    this->FullProcessTerrainExtractFilter = 0;
//    return;
//  }

//  // go ahead and update max refine level for the full refine
//  this->FullProcessTerrainExtractFilter->getProxy()->UpdatePropertyInformation();
//  maxLevel = pqSMAdaptor::getElementProperty(
//               this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("NumberOfLevels"))
//               .toInt() -
//    1;

//  ////////// Extraction Phase ////////////

//  this->LIDARCore->updateProgress(QString("Extracting terrain"), 0);

//  // now, the extraction
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("WriteExtractionResultsToDisk"),
//    true);

//  // pass color and/or info from input to output (if present)?
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("DetermineIntensityAndColor"),
//    this->LIDARPanel->getGUIPanel()->computeColor->isChecked());

//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("ExecuteMode"), 2); // extract
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("MinExtractLevel"), minLevel);
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("MaxExtractLevel"), maxLevel);
//  pqSMAdaptor::setElementProperty(
//    this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"), 0);
//  this->FullProcessTerrainExtractFilter->getProxy()->UpdateVTKObjects();

//  this->LIDARCore->enableAbort(true);
//  vtkSMSourceProxy::SafeDownCast(this->FullProcessTerrainExtractFilter->getProxy())
//    ->UpdatePipeline();
//  this->LIDARCore->enableAbort(false);

//  aborted = pqSMAdaptor::getElementProperty(
//              this->FullProcessTerrainExtractFilter->getProxy()->GetProperty("AbortExecute"))
//              .toInt();

//  bool previewOutput = this->LIDARPanel->getGUIPanel()->previewOnCompletionCheckBox->isChecked();
//  if (aborted)
//  {
//    if (previewOutput)
//    {
//      if (QMessageBox::question(this->LIDARCore->parentWidget(), tr("Preview Aborted Output?"),
//            tr("Preview Output selected but extraction aborted.  Preview partial output (if any)?"),
//            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
//      {
//        previewOutput = false;
//      }
//    }
//    else
//    {
//      QMessageBox::warning(this->LIDARCore->parentWidget(), tr("Terrain Extraction Aborted"),
//        tr("The terrain extraction was aborted."), QMessageBox::Ok);
//    }
//  }
//  else
//  {
//    if (!previewOutput)
//    {
//      if (QMessageBox::question(this->LIDARCore->parentWidget(), tr("Terrain Extraction Complete"),
//            tr("The terrain extraction has completed, but preview was not selected.  Do you want "
//               "to preview the output?"),
//            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
//      {
//        previewOutput = true;
//      }
//    }
//    else
//    {
//      QMessageBox::warning(this->LIDARCore->parentWidget(), tr("Terrain Extraction Complete"),
//        tr("The terrain extraction has completed."), QMessageBox::Ok);
//    }
//  }

//  if (previewOutput)
//  {
//    this->addExtractionOutputToTree(minLevel, maxLevel, scale, autoSaveInfo);
//  }

//  this->LIDARPanel->getGUIPanel()->tabWidget->setEnabled(true);
//  emit this->enableMenuItems(true);
//  // right now not supporting repeat full process
//  builder->destroy(this->FullProcessTerrainExtractFilter);
//  this->FullProcessTerrainExtractFilter = 0;

//  this->LIDARCore->onRequestRender();
//}

//void pqTerrainExtractionManager::addExtractionOutputToTree(
//  int minLevel, int maxLevel, double initialScale, QFileInfo& autoSaveInfo)
//{
//  QTreeWidgetItem* baseNode =
//    new QTreeWidgetItem(this->LIDARPanel->getGUIPanel()->extractionTreeWidget->invisibleRootItem());
//  baseNode->setText(0, "Extract Result");
//  baseNode->setIcon(1, *this->IconVisible);
//  baseNode->setData(1, Qt::UserRole, 1);

//  vtksys::Glob glob;
//  QString extension(autoSaveInfo.completeSuffix());

//  double scale = initialScale;
//  QTreeWidgetItem* initialVisibleItem = 0;
//  for (int i = minLevel; i < maxLevel; i++)
//  {
//    // total points for this level; estimate based on bounds and scale
//    double totalPointsEstimate =
//      ceil(this->InputDims[0] / scale) * ceil(this->InputDims[1] / scale);
//    scale *= sqrt(2.0); // increase scale for next level

//    QChar fillChar = '0';
//    QString globString = autoSaveInfo.absolutePath() + "/" + autoSaveInfo.baseName() +
//      QString("_%1*.").arg(i, 2, 10, fillChar) + extension;
//    glob.FindFiles(globString.toStdString().c_str());
//    std::vector<std::string> files = glob.GetFiles();
//    if (files.size() == 0)
//    {
//      continue;
//    }

//    QTreeWidgetItem* levelNode = new QTreeWidgetItem(baseNode);
//    levelNode->setText(0, QString("Level %1").arg(i));
//    levelNode->setIcon(1, *this->IconInvisible);
//    levelNode->setData(1, Qt::UserRole, 0);

//    // which level should be visibly initially... closest (but less than) our
//    // max of 100000 points
//    if (!initialVisibleItem && totalPointsEstimate < 100000)
//    {
//      initialVisibleItem = levelNode;
//    }

//    std::vector<std::string>::const_iterator file;
//    int levelPieceId = 0;
//    QTreeWidgetItem* leafNode;
//    qulonglong leafTotalPointEstimate;
//    for (file = files.begin(); file != files.end(); file++)
//    {
//      if (files.size() == 1)
//      {
//        leafNode = levelNode;
//        leafTotalPointEstimate = totalPointsEstimate;
//      }
//      else
//      {
//        leafNode = new QTreeWidgetItem(levelNode);
//        leafNode->setText(0, QString::number(levelPieceId++));
//        leafNode->setIcon(1, *this->IconInvisible);
//        leafNode->setData(1, Qt::UserRole, 0);
//        leafTotalPointEstimate = totalPointsEstimate / files.size();
//      }
//      QVariant vdata;
//      vdata.setValue(0);
//      leafNode->setData(0, Qt::UserRole, vdata);
//      QString filename = file->c_str();
//      vdata.setValue(filename);
//      leafNode->setData(2, Qt::UserRole, vdata);
//      vdata.setValue(leafTotalPointEstimate);
//      leafNode->setData(3, Qt::UserRole, vdata);
//    }
//  }

//  baseNode->setExpanded(true);

//  // force the read ofthe one level we're going to make visible
//  if (initialVisibleItem)
//  {
//    this->onItemClicked(initialVisibleItem, 1);
//    this->LIDARPanel->getGUIPanel()->extractionTreeWidget->setCurrentItem(initialVisibleItem);
//  }
//}

//pqDataRepresentation* pqTerrainExtractionManager::createPreviewRepresentation(
//  QString& filename)
//{
//  QFileInfo fileInfo(filename);
//  QString extension(fileInfo.completeSuffix().toLower());
//  QStringList filenameSL;
//  filenameSL << filename;

//  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
//  builder->blockSignals(true);
//  pqPipelineSource* readerSource;
//  double minPt[3] = { 0, 0, 0 }, maxPt[3] = { 0, 0, 0 };
//  if (extension == "pts" || extension == "xyz")
//  {
//    readerSource = builder->createReader(
//      "sources", "LIDARReader", filenameSL, this->LIDARCore->getActiveServer());

//    vtkSMPropertyHelper(readerSource->getProxy(), "MaxNumberOfPoints").Set(100000);
//    vtkSMPropertyHelper(readerSource->getProxy(), "LimitToMaxNumberOfPoints").Set(1);
//    readerSource->getProxy()->UpdateVTKObjects();
//    vtkSMSourceProxy::SafeDownCast(readerSource->getProxy())->UpdatePipeline();
//    readerSource->getProxy()->UpdatePropertyInformation();
//    double* dataBounds =
//      vtkSMDoubleVectorProperty::SafeDownCast(readerSource->getProxy()->GetProperty("DataBounds"))
//        ->GetElements();
//    minPt[2] = dataBounds[4];
//    maxPt[2] = dataBounds[5];
//  }
//  else
//  {
//    readerSource = builder->createReader(
//      "sources", "CMBGeometryReader", filenameSL, this->LIDARCore->getActiveServer());
//    pqPipelineSource* polyDataStatsFilter =
//      builder->createFilter("filters", "PolyDataStatsFilter", readerSource);
//    vtkSMSourceProxy::SafeDownCast(polyDataStatsFilter->getProxy())->UpdatePipeline();
//    minPt[2] = pqSMAdaptor::getMultipleElementProperty(
//                 polyDataStatsFilter->getProxy()->GetProperty("GeometryBounds"), 4)
//                 .toDouble();
//    maxPt[2] = pqSMAdaptor::getMultipleElementProperty(
//                 polyDataStatsFilter->getProxy()->GetProperty("GeometryBounds"), 5)
//                 .toDouble();
//    builder->destroy(polyDataStatsFilter);
//  }
//  builder->blockSignals(false);

//  pqPipelineSource* elevationSource =
//    builder->createFilter("filters", "LIDARElevationFilter", readerSource);

//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("LowPoint"), 0, minPt[0]);
//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("LowPoint"), 1, minPt[1]);
//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("LowPoint"), 2, minPt[2]);
//  elevationSource->getProxy()->UpdateProperty("LowPoint", true);
//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("HighPoint"), 0, maxPt[0]);
//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("HighPoint"), 1, maxPt[1]);
//  pqSMAdaptor::setMultipleElementProperty(
//    elevationSource->getProxy()->GetProperty("HighPoint"), 2, maxPt[2]);
//  elevationSource->getProxy()->UpdateProperty("HighPoint", true);

//  vtkSMSourceProxy::SafeDownCast(elevationSource->getProxy())->UpdatePipeline();

//  pqPipelineSource* pdSource =
//    builder->createSource("sources", "HydroModelPolySource", this->LIDARCore->getActiveServer());

//  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
//    ->CopyData(vtkSMSourceProxy::SafeDownCast(elevationSource->getProxy()));
//  pdSource->updatePipeline();

//  builder->destroy(elevationSource);
//  builder->destroy(readerSource);

//  this->PDSources.push_back(pdSource);
//  pqDataRepresentation* representation = builder->createDataRepresentation(
//    pdSource->getOutputPort(0), this->LIDARCore->activeRenderView(), "GeometryRepresentation");
//  int mapScalars = 0;
//  std::string colorMode = "Color";
//  if (this->LIDARPanel->getGUIPanel()->extractionElevationFilter->isChecked())
//  {
//    mapScalars = 1;
//    colorMode = "Elevation";
//  }
//  vtkSMPropertyHelper(representation->getProxy(), "MapScalars").Set(mapScalars);
//  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
//    representation->getProxy(), colorMode.c_str(), vtkDataObject::POINT);

//  return representation;
//}

//bool pqTerrainExtractionManager::setSubTreeVisibility(
//  QTreeWidgetItem* item, bool visible, QIcon* icon)
//{
//  item->setIcon(1, *icon);
//  item->setData(1, Qt::UserRole, visible);

//  bool representationLoaded = false;
//  // loop through children, setting state
//  QTreeWidgetItem* child;
//  for (int i = 0; i < item->childCount(); i++)
//  {
//    child = item->child(i);

//    if (child->childCount())
//    {
//      if (this->setSubTreeVisibility(child, visible, icon))
//      {
//        representationLoaded = true;
//      }
//    }
//    else
//    {
//      child->setIcon(1, *icon);
//      child->setData(1, Qt::UserRole, visible);

//      // do something with rep
//      pqDataRepresentation* rep =
//        static_cast<pqDataRepresentation*>(child->data(0, Qt::UserRole).value<void*>());
//      if (!rep && visible)
//      {
//        QString fileName = child->data(2, Qt::UserRole).value<QString>();
//        rep = this->createPreviewRepresentation(fileName);
//        representationLoaded = true;
//        QVariant vdata;
//        vdata.setValue(static_cast<void*>(rep));
//        child->setData(0, Qt::UserRole, vdata);
//      }
//      if (rep)
//      {
//        rep->setVisible(visible);
//      }
//    }
//  }

//  return representationLoaded;
//}

//void pqTerrainExtractionManager::makeAllInvisible()
//{
//  QTreeWidgetItem* rootNode =
//    this->LIDARPanel->getGUIPanel()->extractionTreeWidget->invisibleRootItem();

//  for (int i = 0; i < rootNode->childCount(); i++)
//  {
//    this->setSubTreeVisibility(rootNode->child(i), false, this->IconInvisible);
//  }
//}

//void pqTerrainExtractionManager::clearTree()
//{
//  this->makeAllInvisible();
//  this->destroyTreeRepresentations(
//    this->LIDARPanel->getGUIPanel()->extractionTreeWidget->invisibleRootItem());

//  this->LIDARPanel->getGUIPanel()->extractionTreeWidget->blockSignals(true);
//  this->LIDARPanel->getGUIPanel()->extractionTreeWidget->clear();
//  this->LIDARPanel->getGUIPanel()->extractionTreeWidget->blockSignals(false);

//  //clear the number of points in the selected index of the tree
//  this->LIDARPanel->getGUIPanel()->numPointsLabel->clear();
//  this->LIDARPanel->getGUIPanel()->numActualPointsLabel->clear();
//}

//void pqTerrainExtractionManager::destroyTreeRepresentations(QTreeWidgetItem* treeNode)
//{
//  if (treeNode->childCount())
//  {
//    for (int i = 0; i < treeNode->childCount(); i++)
//    {
//      this->destroyTreeRepresentations(treeNode->child(i));
//    }
//  }
//  else
//  {
//    pqDataRepresentation* rep =
//      static_cast<pqDataRepresentation*>(treeNode->data(0, Qt::UserRole).value<void*>());
//    if (rep)
//    {
//      pqApplicationCore::instance()->getObjectBuilder()->destroy(rep);
//    }
//  }
//}

//void pqTerrainExtractionManager::onSaveRefineResultsChange(bool change)
//{
//  this->SaveRefineResults = change;
//}
