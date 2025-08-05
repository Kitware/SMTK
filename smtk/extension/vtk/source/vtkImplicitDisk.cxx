//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkImplicitDisk.h"

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

vtkStandardNewMacro(vtkImplicitDisk);

vtkImplicitDisk::vtkImplicitDisk()
  : CenterPoint{ 0, 0, 0 }
  , Normal{ 0, 0, 1 }
{
}

vtkImplicitDisk::~vtkImplicitDisk() = default;

void vtkImplicitDisk::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "CenterPoint: (" << this->CenterPoint << ")\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Normal: (" << this->Normal << ")\n";
}

double vtkImplicitDisk::EvaluateFunction(double xx[3])
{
  vtkVector3d pp(xx);
  vtkVector3d delta = this->CenterPoint - pp;
  double halfPlane = delta.Dot(this->Normal);
  delta = delta - halfPlane * this->Normal;
  double outOfDisk = delta.Norm() - this->Radius;
  double outOfPlane = halfPlane < 0. ? -halfPlane : halfPlane;
  // outOfPlane is >= 0.
  // outOfDisk is < 0. if inside, > 0 if outside.
  // The signed distance is a combination of these two distances, with its
  // sign modulated by the sign of outOfDisk.
  double signedDist =
    std::copysign(std::sqrt(outOfDisk * outOfDisk + outOfPlane * outOfPlane), outOfDisk);
  return signedDist;
}

void vtkImplicitDisk::EvaluateGradient(double xx[3], double gg[3])
{
  vtkVector3d pp(xx);
  vtkVector3d delta = this->CenterPoint - pp;
  double halfPlane = delta.Dot(this->Normal);
  delta = delta - halfPlane * this->Normal;
  double outOfDisk = delta.Norm() - this->Radius;
  // outOfPlane is (0,0,0) on the plane with normal this->Normal passing
  // through this->CenterPoint and unit length pointing away from this
  // plane everywhere else in space.
  vtkVector3d outOfPlane;
  if (halfPlane > 0.)
  {
    outOfPlane = -this->Normal * halfPlane;
  }
  else
  {
    outOfPlane = this->Normal * halfPlane;
  }
  // inPlane = (0,0,0) at CenterPoint, unit length pointing away from disk center elswhere.
  vtkVector3d inPlane = delta.Normalized() * (outOfDisk < 0 ? 0. : -outOfDisk);
  vtkVector3d gradient = inPlane + outOfPlane;
  gg[0] = gradient[0];
  gg[1] = gradient[1];
  gg[2] = gradient[2];
}

bool vtkImplicitDisk::SetRadius(double radius)
{
  if (radius <= 0.0)
  {
    return false;
  }
  this->Radius = radius;
  return true;
}

bool vtkImplicitDisk::SetCenterPoint(const vtkVector3d& pt)
{
  if (pt == this->CenterPoint)
  {
    return false;
  }
  this->CenterPoint = pt;
  return true;
}

bool vtkImplicitDisk::SetNormal(const vtkVector3d& normal)
{
  auto nn = normal.Normalized();
  if (nn == this->Normal)
  {
    return false;
  }
  this->Normal = nn;
  return true;
}
