//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkOpenCVHelper_h
#define vtkOpenCVHelper_h

#include "vtkCellArray.h"
#include "vtkImageData.h"
#include "vtkImageImport.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "opencv2/imgproc.hpp"

class vtkOpenCVHelper
{
public:
  static bool OpenCVToVTK( cv::Mat const& src, double * origin,
                          double * spacing, vtkImageData* dest )
  {
    if(src.data == NULL )
    {
      return false;
    }
    cv::Mat src_c = src.clone();
    vtkSmartPointer<vtkImageImport> importer = vtkSmartPointer<vtkImageImport>::New();
    if ( !dest )
    {
      return false;
    }
    importer->SetDataSpacing( spacing );
    importer->SetDataOrigin( origin );
    importer->SetWholeExtent(0, src.size().width-1, 0, src.size().height-1, 0, 0 );
    importer->SetDataExtentToWholeExtent();
    importer->SetDataScalarTypeToUnsignedChar();
    importer->SetNumberOfScalarComponents( src.channels() );
    importer->SetImportVoidPointer( src.data );
    importer->Update();
    dest->DeepCopy(importer->GetOutput());
    return true;
  }

  static bool VTKToOpenCV( vtkImageData* src, cv::Mat& dest,
                           bool convert_to_gray = false)
  {
    const int numComponents =  src->GetNumberOfScalarComponents();
    int type = 0;
    if(numComponents == 3) type = CV_8UC3;
    else if(numComponents == 4) type = CV_8UC4;
    else if(numComponents == 1) type = CV_8UC1;

    int dims[3];
    src->GetDimensions(dims);
    dest = cv::Mat(dims[1], dims[0], type, src->GetScalarPointer());
    if(numComponents != 1)
    {
      if(convert_to_gray)
      {
        cv::cvtColor( dest, dest, CV_RGBA2GRAY);
      }
      else
      {
        cv::cvtColor( dest, dest, CV_RGB2BGR);
      }
    }

    return true;
  }

  static bool ExtractContours( cv::Mat const& src, double * origin,
                               double * spacing, int objToContour, vtkPolyData * poly )
  {
    cv::Mat m = src.clone();
    cv::Mat mV = src == objToContour;
    cv::Mat nV = src != objToContour;

    m.setTo(255, mV);
    m.setTo(0, nV);

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(m, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType startId;
    vtkIdType endId;
    vtkIdType ptIDs[2];
    for(unsigned int i = 0; i < contours.size(); ++i)
    {
      std::vector<cv::Point> const& c = contours[i];
      if(c.empty()) continue;
      //std::cout << "Contour i: " << i << " " <<  c.size() << std::endl;
      startId = endId = points->InsertNextPoint(origin[0] + c[0].x*spacing[0],
                                                origin[1] + c[0].y*spacing[1], 0.001);
      //std::cout << "\t" << c[0].x << " " << c[0].y << std::endl;
      for(unsigned int j = 1; j < c.size(); ++j)
      {
        ptIDs[0] = endId;
        ptIDs[1] = endId = points->InsertNextPoint(origin[0] + c[j].x*spacing[0],
                                                   origin[1] + c[j].y*spacing[1], 0.001);
        cells->InsertNextCell(2,ptIDs);
        //std::cout << "\t" << c[j].x << " " << c[j].y << std::endl;
      }
      ptIDs[0] = endId;
      ptIDs[1] = startId;
      cells->InsertNextCell(2,ptIDs);
    }
    
    poly->SetPoints(points);
    poly->SetLines(cells);
    
    return true;
  }

};

#endif
