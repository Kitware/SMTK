//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqGenerateContoursDialog.h"
#include "ui_qtGenerateContoursDialog.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqProgressManager.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqSetName.h"

#include "vtkDataObject.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkPVRenderView.h"
#include "vtkSMColorMapEditorHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTransferFunctionProxy.h"

#include <QDoubleValidator>
#include <QFileInfo>
#include <QIntValidator>
#include <QMessageBox>
#include <QProgressDialog>
#include <QVBoxLayout>

InternalDoubleValidator::InternalDoubleValidator(QObject* parent)
  : QDoubleValidator(parent)
{
}

void InternalDoubleValidator::fixup(QString& input) const
{
  if (input.length() == 0)
  {
    return;
  }

  double v = input.toDouble();
  double tol = 0.0001;
  if (v < this->bottom())
  {
    input = QString::number(this->bottom() + tol);
  }
  else if (v > this->top())
  {
    input = QString::number(this->top() - tol);
  }
}

inline bool internal_COLOR_REP_BY_ARRAY(
  vtkSMProxy* reproxy,
  const char* arrayname,
  int attribute_type,
  bool rescale = true)
{
  bool res = vtkSMColorMapEditorHelper::SetScalarColoring(reproxy, arrayname, attribute_type);
  if (rescale && res && vtkSMColorMapEditorHelper::GetUsingScalarColoring(reproxy))
  {
    vtkSMPropertyHelper inputHelper(reproxy->GetProperty("Input"));
    vtkSMSourceProxy* inputProxy = vtkSMSourceProxy::SafeDownCast(inputHelper.GetAsProxy());
    int port = inputHelper.GetOutputPort();
    if (inputProxy)
    {
      vtkPVDataInformation* dataInfo = inputProxy->GetDataInformation(port);
      vtkPVArrayInformation* info = dataInfo->GetArrayInformation(arrayname, attribute_type);
      vtkSMPVRepresentationProxy* pvRepProxy = vtkSMPVRepresentationProxy::SafeDownCast(reproxy);
      if (!info && pvRepProxy)
      {
        vtkPVDataInformation* representedDataInfo = pvRepProxy->GetRepresentedDataInformation();
        info = representedDataInfo->GetArrayInformation(arrayname, attribute_type);
      }
      // make sure we have the requested array before calling rescale TF
      if (info)
      {
        res = vtkSMColorMapEditorHelper::RescaleTransferFunctionToDataRange(
          reproxy, arrayname, attribute_type);
      }
    }
  }
  return res;
}

pqGenerateContoursDialog::pqGenerateContoursDialog(
  pqPipelineSource* imagesource,
  const bool& mapScalars2Colors,
  QWidget* parent,
  Qt::WindowFlags flags)
  : QDialog(parent, flags)
  , ImageSource(imagesource)
{
  this->MainDialog = new QDialog;
  this->InternalWidget = new Ui::qtGenerateContoursDialog;
  this->InternalWidget->setupUi(this->MainDialog);
  QObject::connect(
    this->InternalWidget->generateContoursButton,
    SIGNAL(clicked()),
    this,
    SLOT(generateContours()));
  QObject::connect(
    this->InternalWidget->createContourNodesButton,
    SIGNAL(clicked()),
    this,
    SLOT(onAccecptContours()));
  QObject::connect(this->InternalWidget->cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
  QObject::connect(
    this->InternalWidget->imageOpacitySlider,
    SIGNAL(valueChanged(int)),
    this,
    SLOT(onOpacityChanged(int)));

  this->InternalWidget->createContourNodesButton->setEnabled(false);
  this->InternalWidget->generateContoursBox->setEnabled(false);

  pqProgressManager* progress_manager = pqApplicationCore::instance()->getProgressManager();
  QObject::connect(
    progress_manager,
    SIGNAL(progress(const QString&, int)),
    this,
    SLOT(updateProgress(const QString&, int)));
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setWindowTitle(QString("Loading Image"));
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();

  // validator for contour value
  this->ContourValidator = new InternalDoubleValidator(parent);
  this->ContourValidator->setNotation(QDoubleValidator::StandardNotation);
  this->ContourValidator->setDecimals(5);
  this->InternalWidget->contourValue->setValidator(this->ContourValidator);

  QDoubleValidator* minLineLengthValidator = new InternalDoubleValidator(parent);
  minLineLengthValidator->setBottom(0);
  this->InternalWidget->minimumLineLength->setValidator(minLineLengthValidator);

  this->InternalWidget->relativeLineLengthCheckbox->setChecked(true);
  this->InternalWidget->minimumLineLength->setText("5");

  // re-set after loading the image
  this->InternalWidget->contourValue->setText("0");

  if (imagesource)
  {
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

    // setup the render widget
    this->RenderView = qobject_cast<pqRenderView*>(
      builder->createView(pqRenderView::renderViewType(), imagesource->getServer()));
    vtkSMPropertyHelper(this->RenderView->getRenderViewProxy(), "InteractionMode")
      .Set(vtkPVRenderView::INTERACTION_MODE_2D);

    QVBoxLayout* vboxlayout = new QVBoxLayout(this->InternalWidget->renderFrame);
    vboxlayout->setMargin(0);
    vboxlayout->addWidget(this->RenderView->widget());

    this->ContourRepresentation = nullptr;
    this->ContourSource = nullptr;
    this->CleanPolyLines = nullptr;

    this->ContourValue = VTK_FLOAT_MAX;
    this->MinimumLineLength = -1;
    this->UseRelativeLineLength = false;
    QObject::connect(
      this->InternalWidget->contourValue,
      SIGNAL(textChanged(const QString&)),
      this,
      SLOT(updateContourButtonStatus()));
    QObject::connect(
      this->InternalWidget->minimumLineLength,
      SIGNAL(textChanged(const QString&)),
      this,
      SLOT(updateContourButtonStatus()));
    QObject::connect(
      this->InternalWidget->relativeLineLengthCheckbox,
      SIGNAL(stateChanged(int)),
      this,
      SLOT(updateContourButtonStatus()));

    // Set up camera reset button
    QObject::connect(
      this->InternalWidget->resetCamera, SIGNAL(clicked()), this, SLOT(resetCamera()));

    // setup Parallel projection and 2D manipulation
    pqSMAdaptor::setElementProperty(
      this->RenderView->getProxy()->GetProperty("CameraParallelProjection"), 1);
    // paraview default 2d manipulators
    const int TwoDManipulators[9] = { 1, 3, 2, 2, 2, 6, 3, 1, 4 };
    vtkSMProxy* viewproxy = this->RenderView->getProxy();
    vtkSMPropertyHelper(viewproxy->GetProperty("Camera2DManipulators")).Set(TwoDManipulators, 9);
    viewproxy->UpdateVTKObjects();

    pqDataRepresentation* imagerep =
      builder->createDataRepresentation(imagesource->getOutputPort(0), this->RenderView);
    if (imagerep)
    {
      // always display the image in Slice representation type
      vtkSMPropertyHelper(imagerep->getProxy(), "Representation").Set("Slice");
      this->ImageRepresentation = imagerep;
      this->onMapScalars(mapScalars2Colors ? 1 : 0);
      this->InternalWidget->mapScalarsCheck->setChecked(mapScalars2Colors);
    }

    QObject::connect(
      this->InternalWidget->mapScalarsCheck,
      SIGNAL(stateChanged(int)),
      this,
      SLOT(onMapScalars(int)));

    // create the image mesh, which will be used for generating contours
    this->ImageMesh = builder->createFilter("polygon_filters", "StructedToMesh", imagesource);
    vtkSMPropertyHelper(this->ImageMesh->getProxy(), "UseScalerForZ").Set(0);
    this->ImageMesh->getProxy()->UpdateVTKObjects();
    this->ImageMesh->getSourceProxy()->UpdatePipeline();
  }

  this->InternalWidget->generateContoursBox->setEnabled(true);
  this->RenderView->resetCamera();
  this->RenderView->forceRender();
  // see pqProxyInformationWidget
  if (this->ImageMesh)
  {
    int extents[6];
    // The extents info will only be available from the original image source used for image representation
    imagesource->getOutputPort(0)->getDataInformation()->GetExtent(extents);
    this->InternalWidget->imageDimensionsLabel->setText(QString("Dimensions:   %1 x %2")
                                                          .arg(extents[1] - extents[0] + 1)
                                                          .arg(extents[3] - extents[2] + 1));

    vtkPVDataInformation* meshInfo = this->ImageMesh->getOutputPort(0)->getDataInformation();
    vtkPVDataSetAttributesInformation* pointInfo = meshInfo->GetPointDataInformation();
    double range[2] = { 0, 0 };
    if (pointInfo->GetNumberOfArrays() > 0)
    {
      vtkPVArrayInformation* arrayInfo = pointInfo->GetArrayInformation(0);
      if (arrayInfo->GetNumberOfComponents() == 1)
      {
        arrayInfo->GetComponentRange(0, range);
      }
    }

    this->InternalWidget->imageScalarRangeLabel->setText(
      QString("Scalar Range:   %1, %2").arg(range[0]).arg(range[1]));
  }
  delete this->Progress;
  this->Progress = nullptr;

  // not ready to enable abort
  //QObject::connect(this->ProgressWidget, SIGNAL(abortPressed()),
  //                 progress_manager, SLOT(triggerAbort()));
  //QObject::connect(progress_manager, SIGNAL(abort()),
  //                 this, SLOT(abort()));
}

pqGenerateContoursDialog::~pqGenerateContoursDialog()
{
  delete this->InternalWidget;
  delete MainDialog;
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  if (this->ContourRepresentation)
  {
    builder->destroy(this->ContourRepresentation);
  }
  if (this->CleanPolyLines)
  {
    builder->destroy(this->CleanPolyLines);
    this->CleanPolyLines = nullptr;
  }
  if (this->ContourSource)
  {
    builder->destroy(this->ContourSource);
    this->ContourSource = nullptr;
  }
  if (this->ImageMesh)
  {
    builder->destroy(this->ImageMesh);
    this->ImageMesh = nullptr;
  }
  if (this->ImageSource)
  {
    builder->destroy(this->ImageSource);
    this->ImageSource = nullptr;
  }
}

int pqGenerateContoursDialog::exec()
{
  return this->MainDialog->exec();
}

void pqGenerateContoursDialog::generateContours()
{
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();
  this->ProgressMessage = "Computing Contours";
  this->updateProgress(this->ProgressMessage, 0);
  this->disableWhileProcessing();

  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

  // save values used for this computation, so can know whether reecute is necessary
  this->ContourValue = this->InternalWidget->contourValue->text().toDouble();
  this->MinimumLineLength = this->InternalWidget->minimumLineLength->text().toDouble();
  this->UseRelativeLineLength = this->InternalWidget->relativeLineLengthCheckbox->isChecked();

  if (!this->ContourSource)
  {
    this->ContourSource = builder->createFilter("filters", "Contour", this->ImageMesh);
  }
  vtkSMPropertyHelper(this->ContourSource->getProxy(), "ContourValues").Set(this->ContourValue);
  this->ContourSource->getProxy()->UpdateVTKObjects();

  // connects lines up and get rid of short lines (with current hard coded setting, lines
  // less than 5 times the average line length are discarded)
  if (!this->CleanPolyLines)
  {
    this->CleanPolyLines =
      builder->createFilter("polygon_filters", "CleanPolylines", this->ContourSource);
  }
  vtkSMPropertyHelper(this->CleanPolyLines->getProxy(), "UseRelativeLineLength")
    .Set(this->UseRelativeLineLength);
  vtkSMPropertyHelper(this->CleanPolyLines->getProxy(), "MinimumLineLength")
    .Set(this->MinimumLineLength);
  this->CleanPolyLines->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->CleanPolyLines->getProxy())->UpdatePipeline();

  pqPipelineSource* polyDataStatsFilter =
    builder->createFilter("polygon_filters", "PolyDataStatsFilter", this->CleanPolyLines);
  vtkSMSourceProxy::SafeDownCast(polyDataStatsFilter->getProxy())->UpdatePipeline();
  polyDataStatsFilter->getProxy()->UpdatePropertyInformation();

  vtkIdType numberOfLines =
    vtkSMPropertyHelper(polyDataStatsFilter->getProxy(), "NumberOfLines").GetAsIdType();

  vtkIdType numberOfPoints =
    vtkSMPropertyHelper(polyDataStatsFilter->getProxy(), "NumberOfPoints").GetAsIdType();

  builder->destroy(polyDataStatsFilter);

  this->InternalWidget->numberOfContoursLabel->setText(
    QString("Number Of Contours: %1").arg(numberOfLines));
  this->InternalWidget->numberOfPointsLabel->setText(
    QString("Number Of Points: %1").arg(numberOfPoints));

  // add as a single representation.... by default it colors by LineIndex, so
  // with a decent color map, should be able to distinguish many the separate contours
  if (!this->ContourRepresentation)
  {
    this->ContourRepresentation = builder->createDataRepresentation(
      this->CleanPolyLines->getOutputPort(0), this->RenderView, "GeometryRepresentation");
    pqSMAdaptor::setElementProperty(
      this->ContourRepresentation->getProxy()->GetProperty("LineWidth"), 2);
    // move contour slightly in front of the image, so no rendering "issues"
    double position[3] = { 0, 0, 0.1 };
    vtkSMPropertyHelper(this->ContourRepresentation->getProxy(), "Position").Set(position, 3);
    this->ContourRepresentation->getProxy()->UpdateVTKObjects();
  }

  this->RenderView->render();
  this->InternalWidget->createContourNodesButton->setEnabled(true);
  this->ProgressMessage = "";
  this->updateProgress(this->ProgressMessage, 0);
  this->InternalWidget->okCancelBox->setEnabled(true);

  // reanble contour box, but disable contour generation button until parameter change
  this->InternalWidget->generateContoursBox->setEnabled(true);
  this->InternalWidget->loadImageFrame->setEnabled(true);
  this->InternalWidget->generateContoursButton->setEnabled(false);
  delete this->Progress;
  this->Progress = nullptr;
}

void pqGenerateContoursDialog::onAccecptContours()
{
  this->Progress = new QProgressDialog(this->MainDialog);
  this->Progress->setMaximum(0.0);
  this->Progress->setMinimum(0.0);

  this->Progress->show();
  this->disableWhileProcessing();

  Q_EMIT this->contoursAccepted(this->CleanPolyLines);
  this->MainDialog->done(QDialog::Accepted);

  delete this->Progress;
  this->Progress = nullptr;
}

void pqGenerateContoursDialog::onCancel()
{
  this->MainDialog->done(QDialog::Rejected);

  // disconnect progressManager????
}

void pqGenerateContoursDialog::close()
{
  this->onCancel();
}

void pqGenerateContoursDialog::updateProgress(const QString& message, int progress)
{
  // Is there any progress being reported?
  if (!this->Progress)
  {
    return;
  }
  this->Progress->setLabelText(message);
  this->Progress->setValue(progress);
  QCoreApplication::processEvents();
}

void pqGenerateContoursDialog::disableWhileProcessing()
{
  this->InternalWidget->loadImageFrame->setEnabled(false);
  this->InternalWidget->generateContoursBox->setEnabled(false);
  this->InternalWidget->okCancelBox->setEnabled(false);
}

void pqGenerateContoursDialog::updateContourButtonStatus()
{
  if (
    this->ContourValue == this->InternalWidget->contourValue->text().toDouble() &&
    this->MinimumLineLength == this->InternalWidget->minimumLineLength->text().toDouble() &&
    this->UseRelativeLineLength == this->InternalWidget->relativeLineLengthCheckbox->isChecked())
  {
    this->InternalWidget->generateContoursButton->setEnabled(false);
  }
  else
  {
    this->InternalWidget->generateContoursButton->setEnabled(true);
  }
}

void pqGenerateContoursDialog::onOpacityChanged(int opacity)
{
  if (this->ImageRepresentation)
  {
    vtkSMPropertyHelper(this->ImageRepresentation->getProxy(), "Opacity")
      .Set(static_cast<double>(opacity) / 100.0);
    this->ImageRepresentation->getProxy()->UpdateVTKObjects();
    this->RenderView->render();
  }
}

void pqGenerateContoursDialog::onMapScalars(int mapScalars2Colors)
{
  if (pqDataRepresentation* imagerep = this->ImageRepresentation)
  {
    if (mapScalars2Colors)
    {
      pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
      // If there is an elevation field on the points then use it.
      internal_COLOR_REP_BY_ARRAY(imagerep->getProxy(), "Elevation", vtkDataObject::POINT);
      vtkSMProxy* lut = builder->createProxy(
        "lookup_tables", "PVLookupTable", this->ImageSource->getServer(), "transfer_functions");
      vtkSMTransferFunctionProxy::ApplyPreset(lut, "CMB Elevation Map 2", false);
      vtkSMPropertyHelper(lut, "ColorSpace").Set(0);
      vtkSMPropertyHelper(lut, "Discretize").Set(0);
      lut->UpdateVTKObjects();
      vtkSMPropertyHelper(imagerep->getProxy(), "LookupTable").Set(lut);
      vtkSMPropertyHelper(imagerep->getProxy(), "SelectionVisibility").Set(0);
    }
    else
    {
      vtkSMPropertyHelper(imagerep->getProxy(), "MapScalars").Set(0);
    }

    imagerep->getProxy()->UpdateVTKObjects();
    imagerep->renderViewEventually();
  }
}

//-----------------------------------------------------------------------------
void pqGenerateContoursDialog::resetCamera()
{
  this->RenderView->resetViewDirection(0., 0., -1., 0., 1., 0.);
}
