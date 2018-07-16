//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "smtk/extension/vtk/reader/vtkGeoSphereTransformLegacy.h"

#include "smtk/extension/vtk/reader/vtkGlobeSourceLegacy.h"

#include "vtkDoubleArray.h"
#include "vtkGeoMath.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"

vtkStandardNewMacro(vtkGeoSphereTransformLegacy);

vtkGeoSphereTransformLegacy::vtkGeoSphereTransformLegacy()
{
  this->ToRectangular = true;
  this->BaseAltitude = 0.0;
}

vtkGeoSphereTransformLegacy::~vtkGeoSphereTransformLegacy() = default;

void vtkGeoSphereTransformLegacy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ToRectangular: " << this->ToRectangular << endl;
  os << indent << "BaseAltitude: " << this->BaseAltitude << endl;
}

void vtkGeoSphereTransformLegacy::Inverse()
{
  this->ToRectangular = !this->ToRectangular;
  this->Modified();
}

void vtkGeoSphereTransformLegacy::InternalTransformPoint(const float in[3], float out[3])
{
  double ind[3];
  double oud[3];
  int i;
  for (i = 0; i < 3; ++i)
    ind[i] = in[i];
  this->InternalTransformPoint(ind, oud);
  for (i = 0; i < 3; ++i)
    out[i] = static_cast<float>(oud[i]);
}

void vtkGeoSphereTransformLegacy::InternalTransformPoint(const double in[3], double out[3])
{
  if (this->ToRectangular)
  {
    vtkGlobeSourceLegacy::ComputeGlobePoint(
      in[0], in[1], vtkGeoMath::EarthRadiusMeters() + in[2] + this->BaseAltitude, out);
  }
  else
  {
    vtkGlobeSourceLegacy::ComputeLatitudeLongitude(const_cast<double*>(in), out[0], out[1]);
    out[2] = vtkMath::Norm(in) - vtkGeoMath::EarthRadiusMeters() - this->BaseAltitude;
  }
}

void vtkGeoSphereTransformLegacy::InternalTransformDerivative(
  const float in[3], float out[3], float derivative[3][3])
{
  double ind[3];
  double oud[3];
  double drd[3][3];
  int i;
  for (i = 0; i < 3; ++i)
    ind[i] = in[i];
  this->InternalTransformDerivative(ind, oud, drd);
  for (i = 0; i < 3; ++i)
  {
    out[i] = static_cast<float>(oud[i]);
    for (int j = 0; j < 3; ++j)
    {
      derivative[i][j] = drd[i][j];
    }
  }
}

void vtkGeoSphereTransformLegacy::InternalTransformDerivative(
  const double in[3], double out[3], double derivative[3][3])
{
  // FIXME: Compute derivatives here
  (void)in;
  (void)out;
  (void)derivative;
}

vtkAbstractTransform* vtkGeoSphereTransformLegacy::MakeTransform()
{
  vtkGeoSphereTransformLegacy* geoTrans = vtkGeoSphereTransformLegacy::New();
  return geoTrans;
}
