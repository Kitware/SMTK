//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkGeoTransformOperator.h"

#include "vtkAbstractArray.h"
#include "vtkAlgorithm.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkGeoSphereTransform.h"
#include "vtkInstantiator.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkGeoTransformOperator);

vtkGeoTransformOperator::vtkGeoTransformOperator()
{
  this->OperateSucceeded = 0;
  this->ConvertFromLatLongToXYZ = false;
  this->LatLongTransform1 = vtkSmartPointer<vtkGeoSphereTransform>::New();
  this->LatLongTransform2 = vtkSmartPointer<vtkTransform>::New();
  this->OriginalModelPoints = vtkSmartPointer<vtkPoints>::New();
}

vtkGeoTransformOperator:: ~vtkGeoTransformOperator()
{
}

void vtkGeoTransformOperator::Operate(
  vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }

  // now fill in the model information.
  vtkDiscreteModel* Model = ModelWrapper->GetModel();
  if(!Model || Model->HasInValidMesh())
    {
    vtkErrorMacro("There is no model or no model mesh.");
    return;
    }

  const DiscreteMesh& mesh = Model->GetMesh();
  vtkIdType numPts = mesh.GetNumberOfPoints();
  double pt[3];
  if (this->ConvertFromLatLongToXYZ)
    {
    if(this->OriginalModelPoints->GetNumberOfPoints() != numPts)
      {
      this->OriginalModelPoints->DeepCopy(mesh.SharePointsPtr());
      }

    for (vtkIdType cc=0; cc < numPts; cc++)
      {
      mesh.GetPoint(cc, pt);
      this->LatLongTransform1->TransformPoint(pt, pt);
      if(cc==0)
        {
        this->LatLongTransform2->Identity();
        double rotationAxis[3], zAxis[3] = {0, 0, 1};
        double tempPt[3] = {pt[0], pt[1], pt[2]};
        vtkMath::Normalize(tempPt);
        vtkMath::Cross(tempPt, zAxis, rotationAxis);
        double angle = vtkMath::DegreesFromRadians( acos(tempPt[2]) );

        this->LatLongTransform2->PreMultiply();
        this->LatLongTransform2->RotateWXYZ(angle, rotationAxis);
        this->LatLongTransform2->Translate(-pt[0], -pt[1], -pt[2]);
        }
      this->LatLongTransform2->TransformPoint(pt, pt);
      mesh.MovePoint(cc, pt);
      }
    }
  else
    {
    if(this->OriginalModelPoints->GetNumberOfPoints()!=numPts)
      {
      vtkErrorMacro("There is no cached model points to convert back to lat-long.");
      return;
      }
    mesh.UpdatePoints(this->OriginalModelPoints);
    }
  ModelWrapper->Modified();
  this->OperateSucceeded = 1;

  return;
}

void vtkGeoTransformOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}
