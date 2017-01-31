//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "imageFeatureExtractorWidget.h"
#include "ui_imageFeatureExtractor.h"

#include "vtkSmartPointer.h"
#include "vtkTesting.h"
#include "vtkNew.h"
#include "vtkPNGReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPNGWriter.h"
#include <vtkImageCanvasSource2D.h>
#include <vtkImageViewer2.h>
#include <vtkRenderer.h>
#include <vtkPropPicker.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyle.h>
#include <vtkAssemblyPath.h>
#include <vtkImageActor.h>
#include <vtkMath.h>
#include <vtkVariant.h>
#include <vtkPropPicker.h>
#include <vtkTextProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageBlend.h>
#include <vtkImageMapper3D.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkContourFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkImageChangeInformation.h>

#include <vtkGDALRasterReader.h>
#include <vtkXMLImageDataReader.h>

#include <vtkXMLImageDataWriter.h>
#include <vtkXMLImageDataReader.h>

#include "smtk/extension/vtk/filter/vtkGrabCutFilter.h"
#include "smtk/extension/vtk/filter/vtkWatershedFilter.h"
#include "smtk/extension/vtk/filter/vtkImageClassFilter.h"
#include "smtk/extension/vtk/filter/vtkCleanPolylines.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>

class vtkDEMImageCanvasSource2D : public vtkImageCanvasSource2D
{
public:
  static vtkDEMImageCanvasSource2D *New()
  {
    return new vtkDEMImageCanvasSource2D;
  }

  void SetOrigin(double * o)
  {
    Origin[0] = o[0];
    Origin[1] = o[1];
    Origin[2] = o[2];
    this->Modified();
    this->ImageData->SetOrigin(o);
  }

  void SetSpacing(double * s)
  {
    Spacing[0] = s[0];
    Spacing[1] = s[1];
    Spacing[2] = s[2];
    this->Modified();
    this->ImageData->SetSpacing(s);
  }

protected:
  int RequestInformation (vtkInformation *vtkNotUsed(request),
                                  vtkInformationVector** vtkNotUsed(inputVector),
                                  vtkInformationVector *outputVector) override
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
                 this->WholeExtent,6);

    outInfo->Set(vtkDataObject::SPACING(), Spacing[0], Spacing[1], Spacing[2] );
    outInfo->Set(vtkDataObject::ORIGIN(), Origin[0], Origin[1], Origin[2]);

    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->ImageData->GetScalarType(),
                                                this->ImageData->GetNumberOfScalarComponents());
    return 1;
  }
  vtkDEMImageCanvasSource2D():vtkImageCanvasSource2D()
  {}
  ~vtkDEMImageCanvasSource2D() override
  {}
  double Origin[3];
  double Spacing[3];
};

class imageFeatureExtractorWidget::Internal
{
public:
  Internal()
  {
    PotAlpha = 0;
    Alpha = 255;
    Radius = 3;
    UpdatePotAlpha = false;
    Forground   = 255;
    Background  =   0;
    PotentialBG =  55;
    PotentialFG = 200;
    shiftButtonPressed = false;
    leftMousePressed = false;

    maskActor   = vtkSmartPointer<vtkImageActor>::New();
    imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    drawing     = vtkSmartPointer<vtkDEMImageCanvasSource2D>::New();
    filterWaterShed = vtkSmartPointer<vtkWatershedFilter>::New();
    filterGrabCuts  = vtkSmartPointer<vtkGrabCutFilter>::New();
    filter          = filterGrabCuts;

    drawing->SetDrawColor(Forground, Forground, Forground, Alpha);

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    imageViewer->SetInputData(image);
    imageViewer->GetRenderer()->ResetCamera();
    imageViewer->GetRenderer()->SetBackground(0,0,0);
    vtkCamera* camera = imageViewer->GetRenderer()->GetActiveCamera();

    vtkSmartPointer<vtkRenderer> maskRender = vtkSmartPointer<vtkRenderer>::New();
    maskRender->SetLayer(1);
    maskRender->AddActor(maskActor);
    maskRender->SetActiveCamera(camera);
    imageViewer->GetRenderWindow()->AddRenderer(maskRender);

    imageViewer->GetRenderWindow()->SetNumberOfLayers(3);

    filterWaterShed->SetInputData(0, image);
    filterWaterShed->SetInputConnection(1, drawing->GetOutputPort());
    filterWaterShed->SetForegroundValue(Forground);
    filterWaterShed->SetBackgroundValue(Background);
    filterWaterShed->SetUnlabeledValue(PotentialBG);

    filterGrabCuts->SetInputData(0, image);
    filterGrabCuts->SetInputConnection(1, drawing->GetOutputPort());
    filterGrabCuts->SetNumberOfIterations(20);
    filterGrabCuts->SetPotentialForegroundValue(PotentialFG);
    filterGrabCuts->SetPotentialBackgroundValue(PotentialBG);
    filterGrabCuts->SetForegroundValue(Forground);
    filterGrabCuts->SetBackgroundValue(Background);

    lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lineActor = vtkSmartPointer<vtkActor>::New();
    lineActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
    lineActor->GetProperty()->SetEdgeColor(1.0, 1.0, 0.0);
    lineMapper->ScalarVisibilityOff();
    lineActor->SetMapper(lineMapper);

    vtkSmartPointer<vtkRenderer> lineRender = vtkSmartPointer<vtkRenderer>::New();
    lineRender->SetLayer(2);
    lineRender->AddActor(lineActor);
    lineRender->SetActiveCamera(camera);
    imageViewer->GetRenderWindow()->AddRenderer(lineRender);

    maskActor->GetMapper()->SetInputConnection(drawing->GetOutputPort());

    propPicker = vtkSmartPointer<vtkPropPicker>::New();
    propPicker->PickFromListOn();
    vtkImageActor* imageActor = imageViewer->GetImageActor();
    propPicker->AddPickList(imageActor);

    imageClassFilter = vtkSmartPointer<vtkImageClassFilter>::New();
    imageClassFilter->SetForegroundValue(Forground);
    imageClassFilter->SetBackgroundValue(Background);

    contFilter = vtkSmartPointer<vtkContourFilter>::New();
    contFilter->SetValue(0,127.5);
    contFilter->ComputeGradientsOn();
    contFilter->ComputeScalarsOff();

    cleanPolyLines = vtkSmartPointer<vtkCleanPolylines>::New();
    cleanPolyLines->SetMinimumLineLength(0);
  }

  int Alpha;
  int PotAlpha;
  int Forground;
  int Background;
  int PotentialBG;
  int PotentialFG;
  int Radius;
  vtkSmartPointer<vtkImageActor> maskActor;
  vtkSmartPointer<vtkDEMImageCanvasSource2D> drawing;
  vtkImageAlgorithm * filter;
  vtkSmartPointer<vtkWatershedFilter> filterWaterShed;
  vtkSmartPointer<vtkGrabCutFilter> filterGrabCuts;
  vtkSmartPointer<vtkPolyDataMapper> lineMapper;
  vtkSmartPointer<vtkActor> lineActor;
  vtkSmartPointer<vtkImageViewer2> imageViewer;
  vtkSmartPointer<vtkPropPicker> propPicker;
  vtkSmartPointer<vtkContourFilter> contFilter;
  vtkSmartPointer<vtkImageClassFilter> imageClassFilter;
  vtkSmartPointer<vtkCleanPolylines> cleanPolyLines;

  bool leftMousePressed;
  bool shiftButtonPressed;
  double LastPt[2];

  bool UpdatePotAlpha;

  void updateAlphas()
  {
    double currentColor[4];
    drawing->GetDrawColor(currentColor);
    vtkImageData * image = drawing->GetOutput();
    int* dims = image->GetDimensions();
    for (int z = 0; z < dims[2]; z++)
    {
      for (int y = 0; y < dims[1]; y++)
      {
        for (int x = 0; x < dims[0]; x++)
        {
          double pclass = image->GetScalarComponentAsDouble(x, y, z, 0);
          if(pclass == this->Forground)
          {
            drawing->SetDrawColor(this->Forground, this->Forground,
                                  this->Forground, this->Alpha);
          }
          else if(pclass == this->Background)
          {
            drawing->SetDrawColor(this->Background, this->Background,
                                  this->Background, this->Alpha);
          }
          else if(pclass == this->PotentialFG)
          {
            drawing->SetDrawColor(this->PotentialFG, this->PotentialFG,
                                  this->PotentialFG, this->PotAlpha);
          }
          else if(pclass == this->PotentialBG)
          {
            drawing->SetDrawColor(this->PotentialBG, this->PotentialBG,
                                  this->PotentialBG, this->PotAlpha);
          }
          else
          {
            std::cout << "Unknown class " << pclass << std::endl;
            continue;
          }
          drawing->DrawPoint(x,y);
        }
      }
    }
    drawing->SetDrawColor(currentColor);
    vtkRenderWindowInteractor *interactor = imageViewer->GetRenderWindow()->GetInteractor();
    interactor->Render();
  }
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkGrabLeftMouseReleasedCallback : public vtkCommand
{
public:
  static vtkGrabLeftMouseReleasedCallback *New()
  {
    return new vtkGrabLeftMouseReleasedCallback;
  }

  void SetData(imageFeatureExtractorWidget::Internal *i)
  {
    this->internal = i;
  }

  void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *) override
  {
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if(!internal->leftMousePressed)
    {
      style->OnLeftButtonUp();
      return;
    }

    if ( internal->shiftButtonPressed )
    {
      internal->leftMousePressed = false;
      internal->shiftButtonPressed = false;
      style->OnLeftButtonUp();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(interactor->GetEventPosition()[0],
                               interactor->GetEventPosition()[1],
                               0.0, renderer);

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };

    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1],
                                image_coordinate[0], image_coordinate[1], internal->Radius);
    style->OnLeftButtonUp();
    internal->leftMousePressed = false;
  }

private:
  imageFeatureExtractorWidget::Internal *internal;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkGrabMouseMoveCallback : public vtkCommand
{
public:
  static vtkGrabMouseMoveCallback *New()
  {
    return new vtkGrabMouseMoveCallback;
  }

  vtkGrabMouseMoveCallback()
  {
    this->internal     = NULL;
  }

  ~vtkGrabMouseMoveCallback() override
  {
    this->internal     = NULL;
  }

  void SetData(imageFeatureExtractorWidget::Internal *i)
  {
    this->internal = i;
  }

  void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *) override
  {
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if(!internal->leftMousePressed || internal->shiftButtonPressed )
    {
      style->OnMouseMove();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(interactor->GetEventPosition()[0],
                               interactor->GetEventPosition()[1],
                               0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode *node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      style->OnMouseMove();
      return;
    }

    interactor->Render();
    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1],
                                image_coordinate[0], image_coordinate[1], internal->Radius);
    internal->LastPt[0] = image_coordinate[0];
    internal->LastPt[1] = image_coordinate[1];
  }
private:
  imageFeatureExtractorWidget::Internal *internal;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkGrabLeftMousePressCallback : public vtkCommand
{
public:
  static vtkGrabLeftMousePressCallback *New()
  {
    return new vtkGrabLeftMousePressCallback;
  }

  vtkGrabLeftMousePressCallback()
  {
    this->internal     = NULL;
  }

  ~vtkGrabLeftMousePressCallback() override
  {
    this->internal     = NULL;
  }

  void SetData(imageFeatureExtractorWidget::Internal *i)
  {
    this->internal = i;
  }

  void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *) override
  {
    internal->leftMousePressed = true;
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    bool shiftKey = interactor->GetShiftKey();
    if(shiftKey && !internal->shiftButtonPressed)
    {
      internal->shiftButtonPressed = true;
    }

    // Pick at the mouse location provided by the interactor
    this->internal->propPicker->Pick(interactor->GetEventPosition()[0],
                                     interactor->GetEventPosition()[1],
                                     0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode *node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      style->OnLeftButtonDown();
      return;
    }

    if( internal->shiftButtonPressed )
    {
      style->OnLeftButtonDown();
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      // Pass the event further on
      style->OnLeftButtonDown();
      return;
    }

    this->internal->drawing->FillBox(image_coordinate[0]-internal->Radius,
                                     image_coordinate[0]+internal->Radius,
                                     image_coordinate[1]-internal->Radius,
                                     image_coordinate[1]+internal->Radius);
    this->internal->leftMousePressed = true;
    this->internal->LastPt[0] = image_coordinate[0];
    this->internal->LastPt[1] = image_coordinate[1];

    interactor->Render();
    style->OnLeftButtonDown();
  }
  
private:
  imageFeatureExtractorWidget::Internal *internal;
};


imageFeatureExtractorWidget::imageFeatureExtractorWidget()
:internal( new imageFeatureExtractorWidget::Internal() )
{
  this->ui = new Ui_imageFeatureExtractor;
  this->ui->setupUi(this);
  connect(this->ui->Accept, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->ui->Cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(this->ui->SaveMask, SIGNAL(clicked()), this, SLOT(saveMask()));
  connect(this->ui->clear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(this->ui->Run, SIGNAL(clicked()), this, SLOT(run()));

  connect(this->ui->NumberOfIter, SIGNAL(valueChanged(int)), this, SLOT(numberOfIterations(int)));
  connect(this->ui->DrawSize, SIGNAL(valueChanged(int)), this, SLOT(pointSize(int)));

  connect(this->ui->MinLandSize, SIGNAL(textChanged(const QString &)),
          this,                  SLOT(setBGFilterSize(const QString &)));
  connect(this->ui->MinWaterSize, SIGNAL(textChanged(const QString &)),
          this,                   SLOT(setFGFilterSize(const QString &)));
  this->ui->MinLandSize->setValidator(new QDoubleValidator(0, 1e50, 7, this->ui->MinLandSize));
  this->ui->MinWaterSize->setValidator(new QDoubleValidator(0, 1e50, 7, this->ui->MinWaterSize));

  connect(this->ui->DrawMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setDrawMode(int)));
  connect(this->ui->Algorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlgorithm(int)));

  connect(this->ui->LabelTrans, SIGNAL(valueChanged(int)), this, SLOT(setTransparency(int)));
  connect(this->ui->DrawPossible, SIGNAL(clicked(bool)), this, SLOT(showPossibleLabel(bool)));

  connect(this->ui->SaveLines, SIGNAL(clicked()), this, SLOT(saveLines()));
  connect(this->ui->LoadLines, SIGNAL(clicked()), this, SLOT(loadLines()));

  this->ui->SaveLines->setEnabled(false);
  this->ui->LoadLines->setEnabled(false);
  this->ui->SaveMask->setEnabled(false);

  this->ui->qvtkWidget->SetRenderWindow(this->internal->imageViewer->GetRenderWindow());
  this->internal->imageViewer->SetupInteractor(
                                          this->ui->qvtkWidget->GetRenderWindow()->GetInteractor());

  vtkSmartPointer<vtkGrabLeftMousePressCallback> pressCallback =
                                          vtkSmartPointer<vtkGrabLeftMousePressCallback>::New();
  pressCallback->SetData(internal);
  vtkSmartPointer<vtkGrabMouseMoveCallback> moveCallback =
                                                vtkSmartPointer<vtkGrabMouseMoveCallback>::New();
  moveCallback->SetData(internal);
  vtkSmartPointer<vtkGrabLeftMouseReleasedCallback> release =
                                        vtkSmartPointer<vtkGrabLeftMouseReleasedCallback>::New();
  release->SetData(internal);

  vtkInteractorStyleImage* imageStyle = this->internal->imageViewer->GetInteractorStyle();
  imageStyle->AddObserver(vtkCommand::MouseMoveEvent, moveCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, pressCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, release);
}

imageFeatureExtractorWidget::~imageFeatureExtractorWidget()
{
  delete internal;
  delete ui;
}

vtkSmartPointer<vtkPolyData> imageFeatureExtractorWidget::getPolydata()
{
  return internal->cleanPolyLines->GetOutput();
}

void imageFeatureExtractorWidget
::setImage(std::string imagefile)
{
  vtkSmartPointer<vtkImageData> inputImage;

  QFileInfo finfo(imagefile.c_str());
  if (finfo.completeSuffix().toLower() == "tif" ||
      finfo.completeSuffix().toLower() == "tiff" ||
      finfo.completeSuffix().toLower() == "dem")
  {
    vtkSmartPointer<vtkGDALRasterReader> source = vtkSmartPointer<vtkGDALRasterReader>::New();
    source->SetFileName(imagefile.c_str());
    source->Update();
    inputImage = source->GetOutput();
  }
  else
  {
    vtkSmartPointer<vtkXMLImageDataReader> source = vtkSmartPointer<vtkXMLImageDataReader>::New();
    source->SetFileName(imagefile.c_str());
    source->Update();
    inputImage = source->GetOutput();
  }

  if(!inputImage)
  {
    return;
  }

  this->ui->SaveLines->setEnabled(true);
  this->ui->LoadLines->setEnabled(true);
  this->ui->SaveMask->setEnabled(true);

  this->internal->imageViewer->SetInputData(inputImage);
  internal->filterGrabCuts->SetInputData(0, inputImage);
  internal->filterWaterShed->SetInputData(0, inputImage);
  internal->drawing->SetNumberOfScalarComponents(4);
  internal->drawing->SetScalarTypeToUnsignedChar();
  internal->drawing->SetExtent(inputImage->GetExtent());
  internal->drawing->SetOrigin(inputImage->GetOrigin());
  internal->drawing->SetSpacing(inputImage->GetSpacing());

  {
    double * s = inputImage->GetSpacing();
    int dims[3];
    inputImage->GetDimensions(dims);
    this->ui->extentX->setText(QString::number(std::abs(s[0]*dims[0])));
    this->ui->extentY->setText(QString::number(std::abs(s[1]*dims[1])));
  }

  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  internal->drawing->SetDrawColor(internal->PotentialBG, internal->PotentialBG,
                                  internal->PotentialBG, this->internal->PotAlpha);
  internal->drawing->FillBox(inputImage->GetExtent()[0], inputImage->GetExtent()[1],
                             inputImage->GetExtent()[2], inputImage->GetExtent()[3]);
  internal->drawing->SetDrawColor(currentColor);
  internal->imageViewer->GetRenderer()->ResetCamera();

  vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  this->internal->contFilter->SetInputData(this->internal->imageClassFilter->GetOutput(0));
  this->internal->contFilter->Update();
  this->internal->cleanPolyLines->SetInputData(this->internal->contFilter->GetOutput());
  this->internal->cleanPolyLines->Update();
  interactor->Render();
}

void imageFeatureExtractorWidget::saveMask()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Current binary mask"),
                                                  "",
                                                  tr("vti file (*.vti)"));
  if(fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());
  vtkImageData * tmp = internal->imageClassFilter->GetOutput();
  writer->SetInputData(tmp);
  writer->Write();
}

void imageFeatureExtractorWidget::run()
{
  this->ui->Run->setText("...Running...");
  this->ui->Run->setEnabled(false);
  QCoreApplication::processEvents();
  this->ui->Run->repaint();
  if(internal->filter == internal->filterGrabCuts.GetPointer())
  {
    internal->filterGrabCuts->DoGrabCut();
  }
  internal->filter->Update();

  internal->imageClassFilter->SetInputData(internal->filter->GetOutput(0));
  internal->imageClassFilter->Update();
  internal->contFilter->SetInputData(this->internal->imageClassFilter->GetOutput());
  internal->contFilter->Update();
  internal->cleanPolyLines->SetInputData(this->internal->contFilter->GetOutput());
  internal->cleanPolyLines->Update();
  internal->lineMapper->SetInputData(internal->cleanPolyLines->GetOutput());
  vtkImageData* updateMask = internal->filter->GetOutput(1);
  vtkImageData* currentMask = internal->drawing->GetOutput();

  int* dims = updateMask->GetDimensions();
  double color[4];
  internal->drawing->GetDrawColor(color);
  for (int z = 0; z < dims[2]; z++)
  {
    for (int y = 0; y < dims[1]; y++)
    {
      for (int x = 0; x < dims[0]; x++)
      {
        double tmpC[] = {updateMask->GetScalarComponentAsFloat(  x, y, z, 0),
                         updateMask->GetScalarComponentAsFloat(  x, y, z, 0),
                         updateMask->GetScalarComponentAsFloat(  x, y, z, 0),
                         currentMask->GetScalarComponentAsFloat( x, y, z, 3) };
        internal->drawing->SetDrawColor(tmpC);
        internal->drawing->DrawPoint(x,y);
      }
    }
  }
  internal->drawing->SetDrawColor(color);
  vtkRenderWindowInteractor *interactor = this->internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
  this->ui->Run->setText("Run");
  this->ui->Run->setEnabled(true);
}

void imageFeatureExtractorWidget::clear()
{
  vtkImageData * image = this->internal->imageViewer->GetInput();
  internal->drawing->SetNumberOfScalarComponents(4);
  internal->drawing->SetScalarTypeToUnsignedChar();
  internal->drawing->SetExtent(image->GetExtent());
  internal->drawing->SetOrigin(image->GetOrigin());
  internal->drawing->SetSpacing(image->GetSpacing());
  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  internal->drawing->SetDrawColor(internal->PotentialBG, internal->PotentialBG,
                                  internal->PotentialBG, this->internal->PotAlpha);
  internal->drawing->FillBox(image->GetExtent()[0], image->GetExtent()[1],
                             image->GetExtent()[2], image->GetExtent()[3]);
  internal->drawing->SetDrawColor(currentColor);

  internal->lineMapper->SetInputData(NULL);

  vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void imageFeatureExtractorWidget::pointSize(int i)
{
  internal->Radius = i;
}

void imageFeatureExtractorWidget::numberOfIterations(int j)
{
  internal->filterGrabCuts->SetNumberOfIterations(j);
}

void imageFeatureExtractorWidget::showPossibleLabel(bool b)
{
  internal->UpdatePotAlpha = b;
  if(internal->UpdatePotAlpha)
  {
    internal->PotAlpha = internal->Alpha;
  }
  else
  {
    internal->PotAlpha = 0;
  }
  double currentColor[4];
  internal->drawing->GetDrawColor(currentColor);
  if(currentColor[0] == internal->PotentialBG || currentColor[0] == internal->PotentialFG)
  {
    currentColor[3] = internal->PotAlpha;
    internal->drawing->SetDrawColor(currentColor);
  }
  internal->updateAlphas();
}

void imageFeatureExtractorWidget::setTransparency(int t)
{
  if(t != internal->Alpha)
  {
    internal->Alpha = t;
    double currentColor[4];
    internal->drawing->GetDrawColor(currentColor);
    if(internal->UpdatePotAlpha)
    {
      internal->PotAlpha = internal->Alpha;
      currentColor[3] = internal->Alpha;
    }
    else if(currentColor[0] == internal->Background || currentColor[0] == internal->Forground)
    {
      currentColor[3] = internal->Alpha;
    }
    internal->drawing->SetDrawColor(currentColor);
    internal->updateAlphas();
  }
}

void imageFeatureExtractorWidget::setFGFilterSize(QString const& f)
{
  internal->imageClassFilter->SetMinFGSize(f.toDouble());
  internal->imageClassFilter->Update();
  internal->contFilter->SetInputData(this->internal->imageClassFilter->GetOutput());
  internal->contFilter->Update();
  internal->cleanPolyLines->SetInputData(this->internal->contFilter->GetOutput());
  internal->cleanPolyLines->Update();
  internal->lineMapper->SetInputData(internal->cleanPolyLines->GetOutput());
  vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void imageFeatureExtractorWidget::setBGFilterSize(QString const& b)
{
  internal->imageClassFilter->SetMinBGSize(b.toDouble());
  internal->imageClassFilter->Update();
  internal->contFilter->SetInputData(this->internal->imageClassFilter->GetOutput());
  internal->contFilter->Update();
  internal->cleanPolyLines->SetInputData(this->internal->contFilter->GetOutput());
  internal->cleanPolyLines->Update();
  internal->lineMapper->SetInputData(internal->cleanPolyLines->GetOutput());
  vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void imageFeatureExtractorWidget::setDrawMode(int m)
{
  switch( m )
  {
    case 1:
      internal->drawing->SetDrawColor(internal->Background, internal->Background,
                                      internal->Background, internal->Alpha);
      break;
    case 0:
      internal->drawing->SetDrawColor(internal->Forground, internal->Forground,
                                      internal->Forground, internal->Alpha);
      break;
    case 2:
      internal->drawing->SetDrawColor(internal->PotentialBG, internal->PotentialBG,
                                      internal->PotentialBG, internal->PotAlpha);
      break;
  }
}

void imageFeatureExtractorWidget::setAlgorithm(int a)
{
  switch(a)
  {
    case 0:
      internal->filter = internal->filterGrabCuts;
      this->ui->NumberOfIter->setEnabled(true);
      break;
    case 1:
      internal->filter = internal->filterWaterShed;
      this->ui->NumberOfIter->setEnabled(false);
      break;
  }
}

void imageFeatureExtractorWidget::saveLines()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Current Lines"),
                                                  "",
                                                  tr("VTI File (*.vti)"));
  if(fileName.isEmpty())
  {
    return;
  }
  vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());

  writer->SetInputData(internal->drawing->GetOutput());
  writer->Write();
}

void imageFeatureExtractorWidget::loadLines()
{
  QString fileName = QFileDialog::getOpenFileName(NULL, tr("Open VTI File"),
                                                  "",
                                                  tr("vti file (*.vti)"));
  if(fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
  reader->SetFileName(fileName.toStdString().c_str());
  reader->Update();
  vtkImageData* updateMask = reader->GetOutput();
  vtkImageData* current = internal->drawing->GetOutput();

  int* dims = updateMask->GetDimensions();
  int* cdims = current->GetDimensions();
  if( dims[2] !=  cdims[2]|| dims[1] !=  cdims[1] || dims[0] != cdims[0])
  {
    QMessageBox::critical(this->parentWidget(), "Line file error",
                          "Not the same size of the loaded image");
    return;
  }
  double color[4];
  internal->drawing->GetDrawColor(color);
  for (int z = 0; z < dims[2]; z++)
  {
    for (int y = 0; y < dims[1]; y++)
    {
      for (int x = 0; x < dims[0]; x++)
      {
        double tmpC[] = {updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                         updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                         updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                         0 };
        internal->drawing->SetDrawColor(tmpC);
        internal->drawing->DrawPoint(x,y);
      }
    }
  }
  internal->drawing->SetDrawColor(color);
  internal->updateAlphas();
}
