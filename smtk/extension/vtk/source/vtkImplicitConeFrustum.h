//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkImplicitConeFrustum_h
#define vtkImplicitConeFrustum_h

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h" // For export macro
#include "vtkImplicitBoolean.h"
#include "vtkNew.h"
#include "vtkVector.h"

class vtkCone;
class vtkPlane;
class vtkTransform;

/**
 * @class vtkImplicitConeFrustum
 * @brief Generate an implicit function whose 0-isocontour
 *        is a right cone frustum.
 *
 * A right cone frustum is an infinite cone truncated by 2
 * planes, each of which is perpendicular to the axis of the cone.
 * vtkImplicitConeFrustum creates a signed distance function for a
 * right cone frustum whose axis lies between 2 given points
 * with the specified radius at each point.
 *
 * Internally, this distance function is composed of the boolean
 * difference between an infinite cone and 2 infinite planes.
 *
 * By default, point 1 (the bottom) lies at the origin with radius 0.5
 * and point 2 (the top) lies at (0,0,1) with radius 0.
 */
class VTKSMTKSOURCEEXT_EXPORT vtkImplicitConeFrustum : public vtkImplicitBoolean
{
public:
  vtkTypeMacro(vtkImplicitConeFrustum, vtkImplicitBoolean);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkImplicitConeFrustum(const vtkImplicitConeFrustum&) = delete;
  vtkImplicitConeFrustum& operator=(const vtkImplicitConeFrustum&) = delete;

  /**
   * Construct with default parameters.
   */
  static vtkImplicitConeFrustum* New();

  //@{
  /**
   * Set/get the radius at the bottom of the cone.
   *
   * It must be non-negative.
   * It may be 0, but not when the TopRadius is also 0.
   */
  virtual bool SetBottomRadius(double radius);
  vtkGetMacro(BottomRadius, double);
  //@}

  //@{
  /**
   * Set/get the radius at the top of the cone.
   *
   * It must be non-negative.
   * It may be 0, but not when the BottomRadius is also 0.
   */
  virtual bool SetTopRadius(double radius);
  vtkGetMacro(TopRadius, double);
  //@}

  //@{
  /**
   * Set/get the bottom point of the cone.
   * The default is 0,0,0.
   */
  virtual bool SetBottomPoint(const vtkVector3d& pt);
  virtual bool SetBottomPoint(double x, double y, double z)
  {
    return this->SetBottomPoint(vtkVector3d(x, y, z));
  }
  vtkGetMacro(BottomPoint, vtkVector3d);
  //@}

  //@{
  /**
   * Set/get the top point of the cone.
   * The default is 0,0,0.
   */
  virtual bool SetTopPoint(const vtkVector3d& pt);
  virtual bool SetTopPoint(double x, double y, double z)
  {
    return this->SetTopPoint(vtkVector3d(x, y, z));
  }
  vtkGetMacro(TopPoint, vtkVector3d);
  //@}

  /// Return the angle (in degrees) between the cone axis and the cone surface.
  ///
  /// This should always return a value in [0,90[.
  /// For cylinders, this will return 0.
  double GetAngle() const;

protected:
  vtkImplicitConeFrustum();
  ~vtkImplicitConeFrustum() override;

  /// Update the internal implicits and mark this object as modified.
  ///
  /// This is invoked by SetBottomPoint, SetBottomRadius, SetTopPoint, SetTopRadius.
  void UpdateImplicit();

  vtkNew<vtkCone> InfiniteCone;
  vtkNew<vtkTransform> ConeTransform;

  vtkVector3d BottomPoint;
  double BottomRadius;
  vtkNew<vtkPlane> BottomPlane;

  vtkVector3d TopPoint;
  double TopRadius;
  vtkNew<vtkPlane> TopPlane;
};

#endif
