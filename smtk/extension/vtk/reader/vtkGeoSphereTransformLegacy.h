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
/**
 * @class   vtkGeoSphereTransformLegacy
 * @brief   A transformation between long-lat-alt and rect coords
 *
 * the cartesian coordinate system is the following (if BaseAltitude is 0),
 * - the origin is at the center of the earth
 * - the x axis goes from the origin to (longtitude=-90,latitude=0), intersection of equator and the meridian passing just east of Galapagos Islands
 * - the y axis goes from the origin to the intersection of Greenwitch meridian and equator (longitude=0,latitude=0)
 * - the z axis goes from the origin to the Geographic North Pole (latitude=90)
 * - therefore the frame is right-handed.
 *
 * This class is a direct port of the deprecated vtkGeoSphereTransform class in
 * VTK. Its name has been changed to avoid collisions with older VTK versions.
*/

#ifndef __smtk_vtkGeoSphereTransformLegacy_h
#define __smtk_vtkGeoSphereTransformLegacy_h

#include "smtk/extension/vtk/reader/vtkSMTKReaderExtModule.h" // For export macro

#include "vtkAbstractTransform.h"

class vtkGeoProjection;

class VTKSMTKREADEREXT_EXPORT vtkGeoSphereTransformLegacy : public vtkAbstractTransform
{
public:
  static vtkGeoSphereTransformLegacy* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkGeoSphereTransformLegacy, vtkAbstractTransform);

  /**
   * Invert the transformation.
   */
  void Inverse() override;

  //@{
  /**
   * This will calculate the transformation without calling Update.
   * Meant for use only within other VTK classes.
   */
  void InternalTransformPoint(const float in[3], float out[3]) override;
  void InternalTransformPoint(const double in[3], double out[3]) override;
  //@}

  //@{
  /**
   * This will transform a point and, at the same time, calculate a
   * 3x3 Jacobian matrix that provides the partial derivatives of the
   * transformation at that point.  This method does not call Update.
   * Meant for use only within other VTK classes.
   */
  void InternalTransformDerivative(
    const float in[3], float out[3], float derivative[3][3]) override;
  void InternalTransformDerivative(
    const double in[3], double out[3], double derivative[3][3]) override;
  //@}

  /**
   * Make another transform of the same type.
   */
  vtkAbstractTransform* MakeTransform() override;

  //@{
  /**
   * If on, this transform converts (long,lat,alt) triples to (x,y,z) as an offset
   * from the center of the earth. Alt, x, y, and z are all be in meters.
   * If off, the transform works in the reverse direction.
   * Initial value is on.
   */
  vtkSetMacro(ToRectangular, bool);
  vtkGetMacro(ToRectangular, bool);
  vtkBooleanMacro(ToRectangular, bool);
  //@}

  //@{
  /**
   * The base altitude to transform coordinates to. This can be useful for transforming
   * lines just above the earth's surface. Default is 0.
   */
  vtkSetMacro(BaseAltitude, double);
  vtkGetMacro(BaseAltitude, double);
  //@}

protected:
  vtkGeoSphereTransformLegacy();
  ~vtkGeoSphereTransformLegacy() override;

  bool ToRectangular;
  double BaseAltitude;

private:
  vtkGeoSphereTransformLegacy(const vtkGeoSphereTransformLegacy&) = delete;
  void operator=(const vtkGeoSphereTransformLegacy&) = delete;
};

#endif // __smtk_vtkGeoSphereTransformLegacy_h
