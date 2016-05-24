//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkGDALRasterPolydataWrapper.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkUniformGrid.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkShortArray.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkXMLDataSetWriter.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>

// GDAL includes
#include <gdal_priv.h>
#include <ogr_spatialref.h>

// C/C++ includes
#include <cassert>
#include <iostream>
#include <vector>

namespace smtk {
  namespace vtk {

vtkStandardNewMacro(vtkGDALRasterPolydataWrapper);

vtkCxxSetObjectMacro(vtkGDALRasterPolydataWrapper, Transform, vtkTransform);

//-----------------------------------------------------------------------------
vtkGDALRasterPolydataWrapper::vtkGDALRasterPolydataWrapper()
:RealNumberOfOutputPoints(-1)
{
  this->SetNumberOfInputPorts(0);
  this->OnRatio = 239;
  this->LimitToMaxNumberOfPoints = false;
  Reader = vtkSmartPointer<vtkGDALRasterReader>::New();
  this->Transform = 0;
  this->TransformOutputData = false;
  this->LimitReadToBounds = false;
}

//-----------------------------------------------------------------------------
vtkGDALRasterPolydataWrapper::~vtkGDALRasterPolydataWrapper()
{
  this->Reader->SetFileName(0);
  this->SetTransform(static_cast<vtkTransform*>(0));
}

//-----------------------------------------------------------------------------
const char* vtkGDALRasterPolydataWrapper::GetProjectionString() const
{
  return this->Reader->GetProjectionString();
}

//-----------------------------------------------------------------------------
const double* vtkGDALRasterPolydataWrapper::GetGeoCornerPoints()
{
  return this->Reader->GetGeoCornerPoints();
}

//-----------------------------------------------------------------------------
int* vtkGDALRasterPolydataWrapper::GetDataExtent()
{
  return this->Reader->GetDataExtent();
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& vtkGDALRasterPolydataWrapper::GetMetaData()
{
  return this->Reader->GetMetaData();
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkGDALRasterPolydataWrapper::GetDomainMetaData(
                                                                const std::string& domain)
{
  return this->Reader->GetDomainMetaData(domain);
}

//-----------------------------------------------------------------------------
const std::string& vtkGDALRasterPolydataWrapper::GetDriverShortName()
{
  return this->Reader->GetDriverShortName();
}

//-----------------------------------------------------------------------------
const std::string& vtkGDALRasterPolydataWrapper::GetDriverLongName()
{
  return this->Reader->GetDriverLongName();
}

vtkIdType vtkGDALRasterPolydataWrapper::GetTotalNumberOfPoints()
{
  return this->Reader->GetNumberOfPoints();
}

void vtkGDALRasterPolydataWrapper::SetFileName(std::string const& fname)
{
  this->FileName = fname;
  this->Reader->SetFileName(FileName.c_str());
  this->Modified();
}

std::string vtkGDALRasterPolydataWrapper::GetFileName()
{
  return this->Reader->GetFileName();
}

#ifdef _MSC_VER
#define strdup _strdup
#endif

//-----------------------------------------------------------------------------
int vtkGDALRasterPolydataWrapper::RequestData(vtkInformation* vtkNotUsed(request),
                                     vtkInformationVector** vtkNotUsed(inputVector),
                                     vtkInformationVector* outputVector)
{
  this->Reader->Update();
  vtkDataObject * vdo = this->Reader->GetOutputDataObject(0);

  vtkImageData * img = vtkImageData::SafeDownCast(vdo);
  vtkUniformGrid * ugrid = NULL;
  if (vdo->IsA("vtkUniformGrid"))
    {
    ugrid = vtkUniformGrid::SafeDownCast(img);
    }
  else
    {
    return 0;
    }

  if (this->LimitReadToBounds)
    {
    this->ReadBBox.Reset();
    this->ReadBBox.SetMinPoint(this->ReadBounds[0], this->ReadBounds[2],
                               this->ReadBounds[4]);
    this->ReadBBox.SetMaxPoint(this->ReadBounds[1], this->ReadBounds[3],
                               this->ReadBounds[5]);
    // the ReadBBox is guaranteed to be "valid", regardless of the whether
    // ReadBounds is valid.  If any of the MonPoint values are greater than
    // the corresponding MaxPoint, the MinPoint component will be set to be
    // the same as the MaxPoint during the SetMaxPoint fn call.
    }

  int step = ceil(sqrt(static_cast<double>(this->OnRatio)));
  if(this->LimitToMaxNumberOfPoints)
    {
    if(this->MaxNumberOfPoints >= this->GetTotalNumberOfPoints())
      {
      step = 1;
      }
    else
      {
      double tmp = sqrt(this->GetTotalNumberOfPoints() / static_cast<double>(this->MaxNumberOfPoints));
      step = ceil(tmp);
      }
    }
  if(step <= 0) step = 1;

  vtkPoints *points = vtkPoints::New();
  vtkCellArray *verts = vtkCellArray::New();
  int xyz[3]; xyz[2] = 0;
  double pt[] = {0,0,0};
  double tranpt[] = {0,0,0};
  double * cp = pt;
  double * ap = pt;
  bool dotrans = this->Transform != NULL && (this->TransformOutputData || this->LimitReadToBounds);
  if(dotrans)
    {
    cp = tranpt;
    }
  if(this->Transform != NULL && this->TransformOutputData)
    {
    ap = tranpt;
    }
  int * extent = img->GetExtent();
  this->RealNumberOfOutputPoints = 0;
  for(int x = extent[0]; x <= extent[1]; x+=step)
    {
    xyz[0] = x;
    for(int y = extent[2]; y <= extent[3]; y+=step)
      {
      xyz[1] = y;
      vtkIdType id = img->ComputePointId(xyz);
      if(ugrid == NULL || ugrid->IsPointVisible(id))
        {
        img->GetPoint(id, pt);
        pt[2] = img->GetScalarComponentAsDouble(x,y,0,0);
        if(dotrans)
          {
          this->Transform->TransformPoint(pt, tranpt);
          }
        if(!this->LimitReadToBounds || this->ReadBBox.ContainsPoint(cp[0], cp[1], cp[2]))
          {
          vtkIdType outputIdx = points->InsertNextPoint( ap );
          verts->InsertNextCell(1, &outputIdx);
          this->RealNumberOfOutputPoints++;
          }
        }
      }
    
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *pdOutput = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  pdOutput->SetPoints( points );
  pdOutput->SetVerts( verts );
  points->UnRegister(this);
  verts->UnRegister(this);

  vtkFieldData* fd = pdOutput->GetFieldData();
  vtkStringArray * strA = vtkStringArray::New();
  strA->SetName("GeoInfoProj4");
  strA->InsertNextValue(this->Reader->GetProjectionString());
  fd->AddArray(strA);

  pdOutput->GetBounds(DataBounds);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkGDALRasterPolydataWrapper::RequestInformation(vtkInformation * vtkNotUsed(request),
                                            vtkInformationVector **vtkNotUsed(inputVector),
                                            vtkInformationVector * vtkNotUsed(outputVector))
{
  if(FileName.empty()) return 0;
  this->Reader->UpdateInformation();
  return 1;
}

//-----------------------------------------------------------------------------
int vtkGDALRasterPolydataWrapper::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
    }
  else
    {
    vtkErrorMacro("Port: " << port << " is not a valid port");
    return 0;
    }
}

//-----------------------------------------------------------------------------
double vtkGDALRasterPolydataWrapper::GetInvalidValue()
{
  return this->Reader->GetInvalidValue();
}

//-----------------------------------------------------------------------------
int vtkGDALRasterPolydataWrapper
::RequestDataObject(vtkInformation *,
                    vtkInformationVector** vtkNotUsed(inputVector) ,
                    vtkInformationVector* outputVector)
{

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(info->Get(vtkDataObject::DATA_OBJECT()));

  if (!output)
    {
    output = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(0, output);
    output->Delete();
    }
  
  return 1;
}

void vtkGDALRasterPolydataWrapper::SetTransform(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}
  } // namespace vtk
} // namespace smtk
