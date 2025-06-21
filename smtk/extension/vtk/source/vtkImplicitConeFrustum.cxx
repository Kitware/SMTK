//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkImplicitConeFrustum.h"

#include "vtkCone.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkTransform.h"
#include "vtkVersionMacros.h"

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 0)
#include "vtkVectorOperators.h"
#endif

#include <cmath>

vtkStandardNewMacro(vtkImplicitConeFrustum);

vtkImplicitConeFrustum::vtkImplicitConeFrustum()
  : BottomPoint{ 0, 0, 0 }
  , TopPoint{ 0, 0, 1 }
{
  // Our output is defined by our subclass, which we configure based on
  // calls that set our member variables.
  this->SetOperationTypeToDifference();
  this->AddFunction(this->InfiniteCone);
  this->AddFunction(this->BottomPlane);
  this->AddFunction(this->TopPlane);
  this->InfiniteCone->SetTransform(this->ConeTransform);
}

vtkImplicitConeFrustum::~vtkImplicitConeFrustum() = default;

void vtkImplicitConeFrustum::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "BottomPoint: (" << this->BottomPoint << ")\n";
  os << indent << "BottomRadius: " << this->BottomRadius << "\n";
  os << indent << "TopPoint: (" << this->TopPoint << ")\n";
  os << indent << "TopRadius: " << this->TopRadius << "\n";
}

bool vtkImplicitConeFrustum::SetBottomRadius(double radius)
{
  if (radius < 0.0 || (radius == 0.0 && this->TopRadius == 0.0) || this->BottomRadius == radius)
  {
    return false;
  }
  this->BottomRadius = radius;
  this->UpdateImplicit();
  return true;
}

bool vtkImplicitConeFrustum::SetTopRadius(double radius)
{
  if (radius < 0.0 || (radius == 0.0 && this->BottomRadius == 0.0) || this->TopRadius == radius)
  {
    return false;
  }
  this->TopRadius = radius;
  this->UpdateImplicit();
  return true;
}

bool vtkImplicitConeFrustum::SetBottomPoint(const vtkVector3d& pt)
{
  if (pt == this->BottomPoint || pt == this->TopPoint)
  {
    return false;
  }
  this->BottomPoint = pt;
  this->UpdateImplicit();
  return true;
}

bool vtkImplicitConeFrustum::SetTopPoint(const vtkVector3d& pt)
{
  if (pt == this->BottomPoint || pt == this->TopPoint)
  {
    return false;
  }
  this->TopPoint = pt;
  this->UpdateImplicit();
  return true;
}

double vtkImplicitConeFrustum::GetAngle() const
{
  vtkVector3d p0(this->BottomPoint);
  vtkVector3d p1(this->TopPoint);
  double height = (p1 - p0).Norm();
  return vtkMath::DegreesFromRadians(atan2(fabs(this->TopRadius - this->BottomRadius), height));
}

void vtkImplicitConeFrustum::UpdateImplicit()
{
  vtkVector3d axis = this->TopPoint - this->BottomPoint;
  vtkVector3d apex = this->BottomPoint;
  double angle = this->GetAngle();

  double length = axis.Norm();
  axis.Normalize();

  if (angle != 0.0)
  {
    double dr = this->TopRadius - this->BottomRadius;
    double ds = -this->BottomRadius * length / dr;
    apex = apex + ds * axis;
  }
  vtkVector3d px{ 1, 0, 0 };
  vtkVector3d py{ 0, 1, 0 };
  vtkVector3d yy = axis.Cross(px);
  if (yy.Norm() < 1e-7)
  {
    yy = axis.Cross(py);
  }
  yy.Normalize();
  vtkVector3d xx = yy.Cross(axis).Normalized();
  // vtkVector3d ss(0., 0., (this->TopRadius - this->BottomRadius) / length);
  const double tfm[16] = { axis[0], xx[0], yy[0], 0., axis[1],  xx[1],    yy[1],    0.,
                           axis[2], xx[2], yy[2], 0., -apex[0], -apex[1], -apex[2], 1. };

  this->InfiniteCone->SetAngle(angle);
  this->ConeTransform->SetMatrix(tfm);
  this->BottomPlane->SetOrigin(this->BottomPoint.GetData());
  this->BottomPlane->SetNormal((-axis).GetData());
  this->TopPlane->SetOrigin(this->TopPoint.GetData());
  this->TopPlane->SetNormal(axis.GetData());

  this->Modified();
}
