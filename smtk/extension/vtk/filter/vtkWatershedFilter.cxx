//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkWatershedFilter.h"
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
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>

vtkStandardNewMacro(vtkWatershedFilter);

vtkWatershedFilter::vtkWatershedFilter()
: ForegroundValue(0),
  BackgroundValue(125),
  UnlabeledValue(255)
{
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(3);
}

int vtkWatershedFilter::RequestData(vtkInformation *vtkNotUsed(request),
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

  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> mask = vtkSmartPointer<vtkImageData>::New();
  mask->DeepCopy(maskVTK);
  image->DeepCopy(inputVTK);

  cv::Mat imageCV, maskCV;
  vtkOpenCVHelper::VTKToOpenCV(image, imageCV);
  vtkOpenCVHelper::VTKToOpenCV(mask, maskCV, /*to_gray*/ true);

#ifdef DEBUG_GUI
  cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
  cv::imshow( "Display window", maskCV );                  // Show our image inside it.
  cv::waitKey(0);
#endif

  for(int i = 0; i < maskCV.rows; i++)
  {
    uchar* Mi = maskCV.ptr<uchar>(i);
    for(int j = 0; j < maskCV.cols; j++)
    {
      if(Mi[j] != ForegroundValue &&
         Mi[j] != BackgroundValue )
      {
        Mi[j] = UnlabeledValue;
      }
    }
  }

  {
    cv::Mat unknown = maskCV == UnlabeledValue;
    cv::Mat BG = maskCV == BackgroundValue;
    cv::Mat FG = maskCV == ForegroundValue;
    maskCV.setTo(0, unknown);
    maskCV.setTo(1, FG);
    maskCV.setTo(2, BG);
  }

  cv::Mat outputLabledImageCV = maskCV.clone();
  outputLabledImageCV.convertTo(outputLabledImageCV, CV_32SC1);
  cv::watershed(imageCV, outputLabledImageCV);
  outputLabledImageCV.convertTo(outputLabledImageCV, CV_8UC1);

  {
    cv::Mat FG = outputLabledImageCV == 1;
    cv::Mat BG = outputLabledImageCV == 2;

    outputLabledImageCV.setTo(ForegroundValue, FG);
    outputLabledImageCV.setTo(BackgroundValue, BG);
  }

  vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
  vtkOpenCVHelper::ExtractContours(outputLabledImageCV, image->GetOrigin(), image->GetSpacing(),
                                   ForegroundValue, poly);

  vtkOpenCVHelper::OpenCVToVTK(outputLabledImageCV, maskVTK->GetOrigin(), maskVTK->GetSpacing(),
                               outputLable);
  outputNext->DeepCopy(maskVTK);

  outputPoly->DeepCopy(poly);
  
  return 1;
}

int vtkWatershedFilter::FillOutputPortInformation(int port, vtkInformation *info)
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
