//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkTexturePointIntensityFilter.h"

#include "vtkCellLocator.h"
#include "vtkDoubleArray.h"
#include "vtkExecutive.h"
#include "vtkGenericCell.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkRegisterPlanarTextureMap.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkTexturePointIntensityFilter);

vtkTexturePointIntensityFilter::vtkTexturePointIntensityFilter()
{
  this->Translation[0] = this->Translation[1] = this->Translation[2] = 0.0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0.0;
  this->Scale[0] = this->Scale[1] = this->Scale[2] = 1.0;
  this->Transform = vtkTransform::New();
  this->Locator = vtkCellLocator::New();
  this->SetNumberOfInputPorts(2);
  this->TestPoint[0] = this->TestPoint[1] = this->TestPoint[2] = 0.0;
  this->Intensity = 0.0;
  this->TransformInverse = nullptr;
}

vtkTexturePointIntensityFilter::~vtkTexturePointIntensityFilter()
{
  this->Locator->Delete();
  this->Transform->Delete();
}

void vtkTexturePointIntensityFilter::SetTextureData(vtkImageData* input)
{
  this->SetInputData(1, input);
}

vtkImageData* vtkTexturePointIntensityFilter::GetTextureData()
{
  if (this->GetNumberOfInputConnections(2) != 2)
  {
    return nullptr;
  }

  return vtkImageData::SafeDownCast(this->GetExecutive()->GetInputData(1, 0));
}

void vtkTexturePointIntensityFilter::SetTextureDataConnection(vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

int vtkTexturePointIntensityFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* vtkNotUsed(outputVector))
{
  // get the info and input data
  vtkPolyData* inputPD = vtkPolyData::SafeDownCast(this->GetInput(0));
  vtkImageData* imageData = vtkImageData::SafeDownCast(this->GetInput(1));

  // input must either have TCoords or field data set by
  // vtkRegisterPlanarTextureMap
  vtkDoubleArray* sRange =
    vtkDoubleArray::SafeDownCast(inputPD->GetFieldData()->GetArray("SRange"));
  vtkDoubleArray* tRange =
    vtkDoubleArray::SafeDownCast(inputPD->GetFieldData()->GetArray("TRange"));
  vtkDoubleArray* sMap = vtkDoubleArray::SafeDownCast(inputPD->GetFieldData()->GetArray("SMap"));
  vtkDoubleArray* tMap = vtkDoubleArray::SafeDownCast(inputPD->GetFieldData()->GetArray("TMap"));
  bool useRegisterPlanarTextureMap = true;
  vtkDataArray* tCoords;
  if (!sRange || !tRange || !sMap || !tMap)
  {
    useRegisterPlanarTextureMap = false;
    tCoords = inputPD->GetPointData()->GetTCoords();
    if (!tCoords)
    {
      vtkErrorMacro(
        "Input polydata must have texture coordinates or field "
        << "data setup by vtkRegisterPlanarTextureMap!");
      return VTK_ERROR;
    }
  }

  //   // See if we need to update the transform
  //   if (this->BuildTime < this->MTime)
  //     {
  this->Transform->Identity();
  this->Transform->PreMultiply();
  this->Transform->Translate(this->Translation);
  this->Transform->RotateZ(this->Orientation[2]);
  this->Transform->RotateX(this->Orientation[0]);
  this->Transform->RotateY(this->Orientation[1]);
  this->Transform->Scale(this->Scale[0], this->Scale[1], this->Scale[2]);

  this->TransformInverse = this->Transform->GetInverse();
  this->BuildTime.Modified();
  //    }

  double tp[3];

  // Transform the test point to the data's coordinate system
  this->TransformInverse->TransformPoint(this->TestPoint, tp);

  double resultTCoord[2] = { 0, 0 };
  if (useRegisterPlanarTextureMap)
  {
    // assumes x, y is correct for plane we have registered texture map on
    vtkRegisterPlanarTextureMap::ComputeTextureCoordinate(
      tp,
      sRange->GetTuple2(0),
      tRange->GetTuple2(0),
      sMap->GetTuple2(0),
      tMap->GetTuple2(0),
      resultTCoord);
  }
  else
  {
    int subId;
    double dist2;
    vtkIdType cellId;
    double closestPt[3], pCoords[3];

    this->Locator->SetDataSet(inputPD);
    this->Locator->BuildLocator();

    // Now calculate the closest point
    this->Locator->FindClosestPoint(tp, closestPt, cellId, subId, dist2);

    vtkNew<vtkGenericCell> cell;
    inputPD->GetCell(cellId, cell.GetPointer());
    vtkIdList* ptIds = cell->GetPointIds();
    double* tCoord;
    if (cell->GetNumberOfPoints() == 1) // vertex
    {
      tCoord = tCoords->GetTuple2(ptIds->GetId(0));
      resultTCoord[0] = tCoord[0];
      resultTCoord[0] = tCoord[1];
    }
    else // line or poly... use weights to determine texture coordinate
    {
      double* weights = new double[cell->GetNumberOfPoints()];
      cell->EvaluatePosition(closestPt, closestPt, subId, pCoords, dist2, weights);

      for (int i = 0; i < cell->GetNumberOfPoints(); i++)
      {
        tCoord = tCoords->GetTuple2(ptIds->GetId(i));
        resultTCoord[0] += tCoord[0] * weights[i];
        resultTCoord[1] += tCoord[1] * weights[i];
      }
      delete[] weights;
    }
  }

  // convert to image coordinates
  int* dims = imageData->GetDimensions();

  int imageCoordinate[3] = { 0, 0, 0 };
  for (int i = 0; i < 2; i++)
  {
    // just in case
    if (resultTCoord[i] < 0)
    {
      resultTCoord[i] = 0;
    }
    if (resultTCoord[i] > 1)
    {
      resultTCoord[i] = 1;
    }
    imageCoordinate[i] = resultTCoord[i] * (dims[i] - 1);
  }

  this->Intensity = VTK_FLOAT_MAX;
  int numComponents = imageData->GetNumberOfScalarComponents();
  switch (imageData->GetScalarType())
  {
    case VTK_UNSIGNED_CHAR:
    {
      if (numComponents != 3)
      {
        vtkErrorMacro("Image # of components != 3 (as unsigned char) not yet supported");
        return VTK_OK;
      }
      unsigned char* rgb =
        static_cast<unsigned char*>(imageData->GetScalarPointer(imageCoordinate));
      // rgb weighting from search of converting rgb -> intensity
      this->Intensity = 0.2989 * static_cast<double>(rgb[0]) / 255.0 +
        0.5870 * static_cast<double>(rgb[1]) / 255.0 + 0.1140 * static_cast<double>(rgb[2]) / 255.0;

      break;
    }
    default:
    {
      vtkErrorMacro("Image component type not yet supported!");
      return VTK_OK;
    }
  }

  return VTK_OK;
}

void vtkTexturePointIntensityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Test Point: (" << this->TestPoint[0] << ", " << this->TestPoint[1] << ", "
     << this->TestPoint[2] << ")\n";

  os << indent << "Intensity: " << this->Intensity << "\n";
  os << indent << "Translation: (" << this->Translation[0] << ", " << this->Translation[1] << ", "
     << this->Translation[2] << ")\n";
  os << indent << "Orientation: (" << this->Orientation[0] << ", " << this->Orientation[1] << ", "
     << this->Orientation[2] << ")\n";
  os << indent << "Scale: (" << this->Scale[0] << ", " << this->Scale[1] << ", " << this->Scale[2]
     << ")\n";
}

int vtkTexturePointIntensityFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 0);
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
  }
  return 0;
}
