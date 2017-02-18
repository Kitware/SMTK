//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkGrabCutFilter.h"
#include "vtkOpenCVHelper.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkImageImport.h"

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>

vtkStandardNewMacro(vtkGrabCutFilter);

class vtkGrabCutFilter::InternalData
{
public:
  InternalData()
  {
    poly = vtkSmartPointer<vtkPolyData>::New();
    mask = vtkSmartPointer<vtkImageData>::New();
  }
  vtkSmartPointer<vtkPolyData> poly;
  vtkSmartPointer<vtkImageData> mask;
  cv::Mat maskCV;
  cv::Mat outputLabledImageCV;
};

vtkGrabCutFilter::vtkGrabCutFilter()
: NumberOfIterations(12),
  PotentialForegroundValue(25),
  PotentialBackgroundValue(255),
  ForegroundValue(0),
  BackgroundValue(125)
{
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(3);
  RunGrabCuts = false;
  internal = new InternalData;
}

vtkGrabCutFilter::~vtkGrabCutFilter()
{
  delete internal;
}

int vtkGrabCutFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                  vtkInformationVector **inputVector,
                                  vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *maskInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outLabelInfo = outputVector->GetInformationObject(0);
  vtkInformation *outNextIterInfo = outputVector->GetInformationObject(1);
  vtkInformation *outPolyDataInfo = outputVector->GetInformationObject(2);

  // Get the input and ouptut
  vtkImageData *inputVTK = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *maskVTK = vtkImageData::SafeDownCast(maskInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData *outputLable =
                      vtkImageData::SafeDownCast(outLabelInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *outputNext =
                    vtkImageData::SafeDownCast(outNextIterInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData * outputPoly =
                    vtkPolyData::SafeDownCast(outPolyDataInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!RunGrabCuts)
  {
    outputPoly->DeepCopy(this->internal->poly);
    outputLable->DeepCopy(this->internal->mask);
    //Might need to fill outputNext and outputLable if how their use changes
    return 1;
  }

  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> mask = vtkSmartPointer<vtkImageData>::New();
  mask->DeepCopy(maskVTK);
  image->DeepCopy(inputVTK);

  cv::Mat imageCV;
  vtkOpenCVHelper::VTKToOpenCV(image, imageCV);
  vtkOpenCVHelper::VTKToOpenCV(mask, this->internal->maskCV, /*to_gray*/ true);

  for(int i = 0; i < this->internal->maskCV.rows; i++)
  {
    uchar* Mi = this->internal->maskCV.ptr<uchar>(i);
    for(int j = 0; j < this->internal->maskCV.cols; j++)
    {
      if(Mi[j] != PotentialBackgroundValue &&
         Mi[j] != PotentialForegroundValue &&
         Mi[j] != ForegroundValue &&
         Mi[j] != BackgroundValue )
      {
        if(std::abs(Mi[j]-PotentialForegroundValue) < std::abs(Mi[j]-PotentialBackgroundValue))
        {
          Mi[j] = PotentialForegroundValue;
        }
        else
        {
          Mi[j] = PotentialBackgroundValue;
        }
      }
    }
  }

  {
    cv::Mat pBG = this->internal->maskCV == PotentialBackgroundValue;
    cv::Mat pFG = this->internal->maskCV == PotentialForegroundValue;
    cv::Mat BG  = this->internal->maskCV == BackgroundValue;
    cv::Mat FG  = this->internal->maskCV == ForegroundValue;
    this->internal->maskCV.setTo(cv::GC_PR_BGD, pBG);
    this->internal->maskCV.setTo(cv::GC_PR_FGD, pFG);
    this->internal->maskCV.setTo(cv::GC_FGD, FG);
    this->internal->maskCV.setTo(cv::GC_BGD, BG);
  }

  cv::Mat bgdModel(1,65,CV_64F, cv::Scalar::all(0)),
          fgdModel(1,65,CV_64F, cv::Scalar::all(0));
  cv::Rect rect;

  cv::grabCut(imageCV, this->internal->maskCV, rect,
              bgdModel, fgdModel, NumberOfIterations, cv::GC_INIT_WITH_MASK);
  RunGrabCuts = false;

  this->internal->outputLabledImageCV = this->internal->maskCV.clone();

  {
    cv::Mat pBG = this->internal->maskCV == cv::GC_PR_BGD;
    cv::Mat pFG = this->internal->maskCV == cv::GC_PR_FGD;
    cv::Mat BG  = this->internal->maskCV == cv::GC_BGD;
    cv::Mat FG  = this->internal->maskCV == cv::GC_FGD;

    this->internal->maskCV.setTo(PotentialBackgroundValue, pBG);
    this->internal->maskCV.setTo(ForegroundValue, FG);
    this->internal->maskCV.setTo(PotentialForegroundValue, pFG);
    this->internal->maskCV.setTo(BackgroundValue, BG);

    this->internal->outputLabledImageCV.setTo(BackgroundValue, pBG);
    this->internal->outputLabledImageCV.setTo(ForegroundValue, FG);
    this->internal->outputLabledImageCV.setTo(ForegroundValue, pFG);
    this->internal->outputLabledImageCV.setTo(BackgroundValue, BG);
  }

  this->internal->poly = vtkSmartPointer<vtkPolyData>::New();
  vtkOpenCVHelper::ExtractContours(this->internal->outputLabledImageCV, image->GetOrigin(),
                                   image->GetSpacing(), ForegroundValue, this->internal->poly);

  vtkOpenCVHelper::OpenCVToVTK(this->internal->outputLabledImageCV, maskVTK->GetOrigin(),
                               maskVTK->GetSpacing(), this->internal->mask);
  vtkOpenCVHelper::OpenCVToVTK(this->internal->maskCV, maskVTK->GetOrigin(),
                               maskVTK->GetSpacing(), outputNext);

  outputPoly->DeepCopy(this->internal->poly);
  outputLable->DeepCopy(this->internal->mask);

  return 1;
}

int vtkGrabCutFilter::FillOutputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  else if (port == 2)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  return 0;
}
