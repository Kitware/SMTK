//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkDEMToMesh.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUniformGrid.h"
#include "vtkVector.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkObjectFactory.h"

#include <map>
#include <sstream>

vtkStandardNewMacro(vtkDEMToMesh);

vtkDEMToMesh::vtkDEMToMesh()
{
  UseScalerForZ = true;
  SubSampleStepSize = 1;//TODO this should be 1
}

vtkDEMToMesh::~vtkDEMToMesh()
{
}


int vtkDEMToMesh::FillInputPortInformation(int /*port*/, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

int vtkDEMToMesh::RequestData( vtkInformation* vtkNotUsed(req),
                               vtkInformationVector** inputVector,
                               vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData* pdOut = vtkPolyData::GetData(outputVector, 0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!input) return 0;

  if(input->IsA("vtkPolyData"))
  {
    vtkPolyData * pd = vtkPolyData::SafeDownCast(input);
    pdOut->ShallowCopy(pd);
    return 1;
  }

  vtkImageData * img = vtkImageData::SafeDownCast(input);
  vtkUniformGrid * ugrid = NULL;
  if (input->IsA("vtkUniformGrid"))
  {
    ugrid = vtkUniformGrid::SafeDownCast(input);
  }

  if (!img)
  {
    return 0;
  }

  vtkPoints *points = vtkPoints::New();
  vtkDoubleArray * scalers = NULL;
  scalers = vtkDoubleArray::New();
  scalers->SetName("Elevation");
  scalers->SetNumberOfComponents(1);

  points->SetDataTypeToDouble();

  int * extent = img->GetExtent();
  double estimatedNumberOfPoly = (extent[1]-extent[0]+1.0)*(extent[3]-extent[2]+1.0)*2;
  while(estimatedNumberOfPoly/(SubSampleStepSize*SubSampleStepSize)>2000000)
  {
    SubSampleStepSize++;
  }

  int sizex = (extent[1]-extent[0]+1)/SubSampleStepSize + 2;
  int sizey = (extent[3]-extent[2]+1)/SubSampleStepSize + 2;

  std::vector< std::vector< int > > ptsGrid(sizex, std::vector< int >(sizey, -1));

  int xyz[3]; xyz[2] = 0;
  double pt[] = {0,0,0};
  double min = 1e23;
  double max  = -1e23;
  for(int x = extent[0]; x <= extent[1]; x += SubSampleStepSize)
  {
    int i = x/SubSampleStepSize;
    xyz[0] = x;
    for(int y = extent[2]; y <= extent[3]; y += SubSampleStepSize)
    {
      int j = y/SubSampleStepSize;
      xyz[1] = y;
      vtkIdType id = img->ComputePointId(xyz);
      if(ugrid == NULL || ugrid->IsPointVisible(id))
      {
        img->GetPoint(id, pt);
        if(UseScalerForZ)
        {
          pt[2] = img->GetScalarComponentAsDouble(x,y,0,0);
          if(pt[2] < min) min = pt[2];
          if(pt[2] > max) max = pt[2];
        }
        double v[] = {img->GetScalarComponentAsDouble(x,y,0,0)};
        scalers->InsertNextTuple(v);
        ptsGrid[i][j] = points->InsertNextPoint(pt);
      }
    }
  }

  vtkCellArray *cells = vtkCellArray::New();

  for(unsigned int i = 0; i < ptsGrid.size()-1; ++i)
  {
    for(unsigned int j = 0; j < ptsGrid[i].size()-1; ++j)
    {
      int ids[] = { ptsGrid[i][j],   ptsGrid[i][j+1],
                    ptsGrid[i+1][j], ptsGrid[i+1][j+1]};
      if(ids[0] == -1 || ids[1] == -1 || ids[2] == -1 || ids[3] == -1)
      {
        continue;
      }
      if((i%2 == 0 && j%2 == 0)||(i%2 != 0 && j%2 != 0))
      {
        vtkIdType tri1[] = {ids[0],ids[3],ids[1]};
        vtkIdType tri2[] = {ids[0],ids[2],ids[3]};
        cells->InsertNextCell(3, tri1);
        cells->InsertNextCell(3, tri2);
      }
      else
      {
        vtkIdType tri1[] = {ids[0],ids[2],ids[1]};
        vtkIdType tri2[] = {ids[1],ids[2],ids[3]};
        cells->InsertNextCell(3, tri1);
        cells->InsertNextCell(3, tri2);
      }
    }
  }

  pdOut->SetPoints(points);
  points->Delete();

  if(scalers)
  {
    pdOut->GetPointData()->SetScalars(scalers);
    scalers->Delete();
  }

  pdOut->SetPolys(cells);
  cells->Delete();

  return 1;
}

void vtkDEMToMesh::SetUseScalerForZ(int v)
{
  int old = UseScalerForZ;
  UseScalerForZ = v;
  if(old != v) this->Modified();
}
