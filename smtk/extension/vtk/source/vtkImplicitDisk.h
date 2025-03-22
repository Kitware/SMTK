//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkImplicitDisk_h
#define vtkImplicitDisk_h

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h" // For export macro
#include "vtkImplicitFunction.h"
#include "vtkNew.h"
#include "vtkVector.h"

class vtkCone;
class vtkPlane;
class vtkTransform;

/**
 * @class vtkImplicitDisk
 * @brief Generate an implicit function whose 0-isocontour
 *        is a planar disk.
 *
 * vtkImplicitDisk creates a signed distance function for a
 * disk whose center lies at a given point with the specified
 * normal vector and radius.
 *
 * By default, point 1 (the center) lies at the origin with radius 0.5
 * and with a z-normal (0, 0, 1).
 */
class VTKSMTKSOURCEEXT_EXPORT vtkImplicitDisk : public vtkImplicitFunction
{
public:
  vtkTypeMacro(vtkImplicitDisk, vtkImplicitFunction);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkImplicitDisk(const vtkImplicitDisk&) = delete;
  vtkImplicitDisk& operator=(const vtkImplicitDisk&) = delete;

  ///@{
  /**
   * Evaluate our implicit function.
   */
  using vtkImplicitFunction::EvaluateFunction;
  double EvaluateFunction(double xx[3]) override;
  ///@}

  /**
    * Evaluate gradient of boolean combination.
    */
  void EvaluateGradient(double xx[3], double gg[3]) override;

  /**
   * Construct with default parameters.
   */
  static vtkImplicitDisk* New();

  //@{
  /**
   * Set/get the radius at the bottom of the cone.
   *
   * It must be non-negative.
   */
  virtual bool SetRadius(double radius);
  vtkGetMacro(Radius, double);
  //@}

  //@{
  /**
   * Set/get the bottom point of the cone.
   * The default is 0,0,0.
   */
  virtual bool SetCenterPoint(const vtkVector3d& pt);
  virtual bool SetCenterPoint(double x, double y, double z)
  {
    return this->SetCenterPoint(vtkVector3d(x, y, z));
  }
  vtkGetMacro(CenterPoint, vtkVector3d);
  //@}

  //@{
  /**
   * Set/get the top point of the cone.
   * The default is 0,0,0.
   */
  virtual bool SetNormal(const vtkVector3d& pt);
  virtual bool SetNormal(double x, double y, double z)
  {
    return this->SetNormal(vtkVector3d(x, y, z));
  }
  virtual bool SetNormal(double* xyz) VTK_SIZEHINT(3)
  {
    return this->SetNormal(vtkVector3d(xyz[0], xyz[1], xyz[2]));
  }
  vtkGetMacro(Normal, vtkVector3d);
  //@}

protected:
  vtkImplicitDisk();
  ~vtkImplicitDisk() override;

  /// Update the internal implicits and mark this object as modified.
  ///
  /// This is invoked by SetCenterPoint, SetRadius, SetNormal.
  void UpdateImplicit();

  // vtkNew<vtkCone> InfiniteCone;
  // vtkNew<vtkTransform> ConeTransform;

  vtkVector3d CenterPoint;
  double Radius{ 0.5 };
  // vtkNew<vtkPlane> BottomPlane;

  vtkVector3d Normal;
  // vtkNew<vtkPlane> TopPlane;
};

#endif
