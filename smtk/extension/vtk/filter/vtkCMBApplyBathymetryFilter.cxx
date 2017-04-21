//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkCMBApplyBathymetryFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImageData.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUniformGrid.h"
#include "vtkWeakPointer.h"

#include <iostream>
#include <map>
#include <sstream>

vtkStandardNewMacro(vtkCMBApplyBathymetryFilter);

namespace{
  template<class T>
  void flatZValues(T *t, int size, double val)
    {
    for (int i=0; i < size; i+=3,t+=3)
      {
      t[2] = val;
      }
    }
  template<class T>
  void copyArrayZValues(T *t, vtkIdType size,
    std::vector<double>& idToElevation,
    bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow)
    {
    double dtmp;
    for (vtkIdType i=0; i < size; ++i)
      {
      dtmp = static_cast<double>(t[i]);
      dtmp = (useHighLimit && dtmp>eleHigh) ? eleHigh : dtmp;
      dtmp = (useLowLimit && dtmp<eleLow) ? eleLow : dtmp;
      idToElevation[i]=dtmp;
      }
    }
  };

class vtkCMBApplyBathymetryFilter::vtkCmbInternalTerrainInfo
  {
public:
  vtkCmbInternalTerrainInfo(vtkInformation* input, const double &radius, double &invalid,
    bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow);
  ~vtkCmbInternalTerrainInfo()
    {
    this->IdToElevation.clear();
    }
  //returns the average elevation for a circle given the radius
  template<class T>
  T getElevation(T *point);
  void setRadius(const double &r)
    {
    Radius = r;
    }

  std::vector<double> IdToElevation;
protected:
  double Radius;
  double InvalidValue;
  vtkSmartPointer<vtkIncrementalOctreePointLocator> Locator;
  };

vtkCMBApplyBathymetryFilter::vtkCmbInternalTerrainInfo::vtkCmbInternalTerrainInfo(
  vtkInformation* inInfo, const double &radius, double &invalid,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow)
: Radius(radius), InvalidValue(invalid)
{
  //1. Create a set of points while removing all the z values from the points
  //and storing them in the map.
  //2. Create locater of the resulting 2D point set

  if(!inInfo)
    {
    return;
    }
  vtkIdType numPoints = 0;
  vtkPolyData* pd = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUniformGrid* gridInput = vtkUniformGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* imageInput = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (pd)
    {
    numPoints += pd->GetNumberOfPoints();
    }
  else if(imageInput)
    {
    numPoints += imageInput->GetNumberOfPoints();
    }
  if(numPoints <=0)
    {
    return;
    }

  //second iteration is building the point set and elevation mapping
  vtkPoints *inputPoints = NULL;
  vtkPoints *points = vtkPoints::New();
  vtkIdType size, i;
  double p[3];
  double dtmp;
  // Uniform Grids may not have the max number of points
  if (gridInput)
    {
    vtkDataArray *dataArray = gridInput->GetPointData()->GetScalars("Elevation");
    if(!dataArray || dataArray->GetNumberOfTuples() != numPoints)
      {
      return;
      }
    points->SetNumberOfPoints(numPoints);
    this->IdToElevation.resize(numPoints);
    vtkIdType at = 0;
    for (i = 0; i < numPoints; i++)
      {
      if (!gridInput->IsPointVisible(i))
        {
        continue;
        }
      dtmp = dataArray->GetTuple1(i);
      dtmp = (useHighLimit && dtmp>eleHigh) ? eleHigh : dtmp;
      dtmp = (useLowLimit && dtmp<eleLow) ? eleLow : dtmp;
      imageInput->GetPoint(i,p);

      //flatten z
      p[2] = 0.0;
      points->SetPoint(at, p);
      this->IdToElevation[at]=dtmp;
      ++at;
      }
    points->Resize(at);
    this->IdToElevation.resize(at);
    }
  else
    {
    points->SetNumberOfPoints(numPoints);
    if (pd)
      {
      inputPoints = pd->GetPoints();
      size = inputPoints->GetNumberOfPoints();
      points->SetNumberOfPoints(size);
      this->IdToElevation.resize(size);
      for (i=0; i < size; ++i)
        {
        //get the point
        inputPoints->GetPoint(i,p);
        // check elevation limit
        dtmp = (useHighLimit && p[2]>eleHigh) ? eleHigh : p[2];
        dtmp = (useLowLimit && dtmp<eleLow) ? eleLow : dtmp;
        //store the z value & flatten
        this->IdToElevation[i]=dtmp;
        p[2] = 0.0;
        points->SetPoint(i,p);
        }
      }
    else if(imageInput)
      {
      size = imageInput->GetNumberOfPoints();
      points->SetNumberOfPoints(size);
      this->IdToElevation.resize(size);
      vtkDataArray *dataArray = imageInput->GetPointData()->GetScalars("Elevation");
      if(!dataArray || dataArray->GetNumberOfTuples() != size)
        {
        return;
        }

      //store the z value
      if (dataArray->GetDataType() == VTK_FLOAT)
        {
        vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
        copyArrayZValues(floatArray->GetPointer(0),
                         size,this->IdToElevation, useHighLimit, eleHigh, useLowLimit, eleLow);
        }
      else if (dataArray->GetDataType() == VTK_DOUBLE)
        {
        vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
        copyArrayZValues(doubleArray->GetPointer(0),
                         size,this->IdToElevation, useHighLimit, eleHigh, useLowLimit, eleLow);
        }

      for (i=0; i < size; ++i)
        {
        //get the point
        imageInput->GetPoint(i,p);

        //flatten z
        p[2] = 0.0;
        points->SetPoint(i,p);
        }
      }
    }
  vtkPolyData *pointSet = vtkPolyData::New();
  pointSet->SetPoints(points);
  points->FastDelete();

  this->Locator = vtkSmartPointer<vtkIncrementalOctreePointLocator>::New();
  this->Locator->AutomaticOn();
  this->Locator->SetTolerance(0.0);
  this->Locator->SetDataSet(pointSet);
  this->Locator->BuildLocator();
  pointSet->Delete();
}

template<class T>
T vtkCMBApplyBathymetryFilter::vtkCmbInternalTerrainInfo::getElevation(
  T *point)
{
  double dpoint[3];
  dpoint[0] = static_cast<double>(point[0]);
  dpoint[1] = static_cast<double>(point[1]);
  dpoint[2] = 0.0;
  vtkIdList *ids = vtkIdList::New();
  this->Locator->FindPointsWithinRadius(this->Radius,dpoint,ids);
  double sum = 0;
  std::map<vtkIdType,double>::const_iterator it;
  vtkIdType size = ids->GetNumberOfIds();
  for ( vtkIdType i=0; i < size; ++i)
    {
    //average the elevation
    assert(ids->GetId(i) < static_cast<vtkIdType>(this->IdToElevation.size()));
    sum += this->IdToElevation[ids->GetId(i)];
    }
  ids->Delete();

  //handle the zero size use case
  T elev = static_cast<T>((size == 0) ? InvalidValue : sum/size);
  return elev;
}

vtkCMBApplyBathymetryFilter::vtkCMBApplyBathymetryFilter()
{
  this->TerrainInfo = NULL;
  this->SetNumberOfInputPorts(2);
  this->ElevationRadius = 1.0;
  this->FlattenZValues = false;
  this->NoOP = false;
  this->FlatZValue = 0.0;
  this->UseHighestZValue = false;
  this->UseLowestZValue = false;
  this->HighestZValue=this->LowestZValue=0.0;
  this->InvalidValue = 0.0;
}

vtkCMBApplyBathymetryFilter::~vtkCMBApplyBathymetryFilter()
{
  if(this->TerrainInfo)
    {
    delete this->TerrainInfo;
    }
}

int vtkCMBApplyBathymetryFilter::FillInputPortInformation(int port,
                                                          vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  if ( port == 1 )
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

void vtkCMBApplyBathymetryFilter::RemoveInputConnections()
{
  this->SetInputConnection(0, NULL);
}

void vtkCMBApplyBathymetryFilter::RemoveSourceConnections()
{
  this->SetInputConnection(1, NULL);
}

int vtkCMBApplyBathymetryFilter::RequestData(vtkInformation* /*request*/,
                                             vtkInformationVector** inputVector,
                                             vtkInformationVector* outputVector)
{
  // get the info and input data
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkPointSet *pdInput = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPointSet *output = vtkPointSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if(this->NoOP)
    {
    output->ShallowCopy(pdInput);
    return 1;
    }
  vtkPointSet* finalMesh = pdInput->NewInstance();
  finalMesh->DeepCopy(pdInput);
  bool validMesh = false;
  if(this->FlattenZValues)
    {
    validMesh = this->FlattenMesh(finalMesh->GetPoints());
    }
  else
    {
    int numInputs = inputVector[1]->GetNumberOfInformationObjects();
    if(numInputs != 1)
      {
      vtkErrorMacro("No bathymetry input.");
      return 0;
      }

    //Construct the TerrainInfo first
    this->TerrainInfo = new vtkCmbInternalTerrainInfo(
      inputVector[1]->GetInformationObject(0),
      this->ElevationRadius, this->InvalidValue, this->UseHighestZValue,
      this->HighestZValue, this->UseLowestZValue, this->LowestZValue);
    if(!this->TerrainInfo->IdToElevation.empty())
      {
      validMesh = this->ApplyBathymetry(finalMesh->GetPoints());
      }

    delete this->TerrainInfo;
    this->TerrainInfo = NULL;
    }

  if(validMesh)
    {
    output->ShallowCopy( finalMesh );
    }
  finalMesh->Delete();

  //force the progress to finish no matter what.
  this->UpdateProgress(1.0);
  return validMesh;
}

bool vtkCMBApplyBathymetryFilter::FlattenMesh(vtkPoints *points)
{
  bool valid = true;
  vtkDataArray *dataArray = points->GetData();
  int size = points->GetNumberOfPoints() * 3;
  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pt = floatArray->GetPointer(0);
    flatZValues(pt,size,this->FlatZValue);
    }
  else if (dataArray->GetDataType() == VTK_DOUBLE)
    {
    vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
    double *pt = doubleArray->GetPointer(0);
    flatZValues(pt,size,this->FlatZValue);
    }
  else
    {
    valid = false;
    }
  return true;
}

bool vtkCMBApplyBathymetryFilter::ApplyBathymetry(vtkPoints *points)
{
  //get the point data void pointer
  vtkDataArray *dataArray = points->GetData();
  vtkIdType size = points->GetNumberOfPoints();
  if(size == 0)
    {
    return false;
    }
  double pnom = static_cast<double>(size);
  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pos = floatArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = this->TerrainInfo->getElevation(pos);
      this->UpdateProgress(static_cast<double>(i)/pnom);
      }
    }
  else
    {
    vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
    double *pos = doubleArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = this->TerrainInfo->getElevation(pos);
      this->UpdateProgress(static_cast<double>(i)/pnom);
      }
    }
  return true;
}

void vtkCMBApplyBathymetryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "NoOP: " << this->NoOP << std::endl;
  os << indent << "ElevationRadius: " << this->ElevationRadius << std::endl;
  os << indent << "FlatZValue: " << this->FlatZValue << std::endl;
  os << indent << "FlattenZValues: " << this->FlattenZValues << std::endl;
  os << indent << "InvalidValue: " << this->InvalidValue << std::endl;
}
