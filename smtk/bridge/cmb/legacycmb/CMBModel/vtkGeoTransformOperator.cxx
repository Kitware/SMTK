/*=========================================================================
Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkGeoTransformOperator.h"

#include "vtkAbstractArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkAlgorithm.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkGeoSphereTransform.h"
#include "vtkTransform.h"
#include "vtkMath.h"

vtkStandardNewMacro(vtkGeoTransformOperator);

//-----------------------------------------------------------------------------
vtkGeoTransformOperator::vtkGeoTransformOperator()
{
  this->OperateSucceeded = 0;
  this->ConvertFromLatLongToXYZ = false;
  this->LatLongTransform1 = vtkSmartPointer<vtkGeoSphereTransform>::New();
  this->LatLongTransform2 = vtkSmartPointer<vtkTransform>::New();
  this->OriginalModelPoints = vtkSmartPointer<vtkPoints>::New();
}

//-----------------------------------------------------------------------------
vtkGeoTransformOperator:: ~vtkGeoTransformOperator()
{
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void vtkGeoTransformOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}
