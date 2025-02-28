//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkConeFrustum_h
#define vtkConeFrustum_h

#include "smtk/extension/vtk/source/vtkSMTKSourceExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

/**
 * @class   vtkConeFrustum
 * @brief   Generate a polygonal approximation to a cone frustum
 *
 * vtkConeFrustum creates a cone whose axis lies between 2 given points
 * with the specified radius at each point.
 * By default, point 1 (the bottom) lies at the origin with radius 0.5
 * and point 2 (the top) lies at (0,0,1) with radius 0.
 * The resolution specifies the number of points around the top and
 * bottom circles; it must be at least 3 and defaults to 16.
 *
 * Note that unlike many other source filters, this one is *not*
 * intended for use as an input to a glyph filter or glyph mapper.
 * Instead, its purpose is to provide a high-quality visual representation
 * of a right cone frustum.
 * That means it uses more points and cells than is strictly necessary
 * but is able to provide a better visual quality as a result.
 * The result is (more or less) topologically correct even when the
 * apex of the cone is included; many points will be coincident, but
 * will be connected into a consistent manifold that is homeomorphic
 * to a sphere.
 */
class VTKSMTKSOURCEEXT_EXPORT vtkConeFrustum : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkConeFrustum, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkConeFrustum(const vtkConeFrustum&) = delete;
  vtkConeFrustum& operator=(const vtkConeFrustum&) = delete;

  /// An enum indexing data present at each output.
  enum OutputPorts
  {
    SideFace = 0, //!< Triangulation of the conical face.
    BottomFace,   //!< Triangulation of the bottom endcap.
    TopFace,      //!< Triangulation of the top endcap.
    Axis,         //!< Line from the bottom to to the top center-point.
    BottomEdge,   //!< Polyline of the bottom endcap feature edge.
    TopEdge,      //!< Polyline of the top endcap feature edge.
    BottomVertex, //!< A single vertex at the center of the bottom face.
    TopVertex,    //!< A single vertex at the center of the top face.
    NumberOfOutputs
  };

  /**
   * Construct with default parameters.
   */
  static vtkConeFrustum* New();

  //@{
  /**
   * Set/get the radius at the bottom of the cone.
   *
   * It must be non-negative.
   * It may be 0, but not when the TopRadius is also 0.
   */
  vtkSetClampMacro(BottomRadius, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(BottomRadius, double);
  //@}

  //@{
  /**
   * Set/get the radius at the top of the cone.
   *
   * It must be non-negative.
   * It may be 0, but not when the BottomRadius is also 0.
   */
  vtkSetClampMacro(TopRadius, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(TopRadius, double);
  //@}

  //@{
  /**
   * Set/get the bottom point of the cone.
   * The default is 0,0,0.
   */
  vtkSetVector3Macro(BottomPoint, double);
  vtkGetVectorMacro(BottomPoint, double, 3);
  //@}

  //@{
  /**
   * Set/get the top point of the cone.
   * The default is 0,0,0.
   */
  vtkSetVector3Macro(TopPoint, double);
  vtkGetVectorMacro(TopPoint, double, 3);
  //@}

  //@{
  /**
   * Set/get the number of facets used to represent the conical side-face.
   *
   * This defaults to 16 and has a minimum of 3.
   */
  vtkSetClampMacro(Resolution, int, 3, VTK_CELL_SIZE);
  vtkGetMacro(Resolution, int);
  //@}

  //@{
  /**
   * Set/get the desired precision for the output points.
   * vtkAlgorithm::SINGLE_PRECISION - Output single-precision floating point.
   * vtkAlgorithm::DOUBLE_PRECISION - Output double-precision floating point.
   */
  vtkSetMacro(OutputPointsPrecision, int);
  vtkGetMacro(OutputPointsPrecision, int);
  //@}

  /// Return the angle (in degrees) between the cone axis and the cone surface.
  ///
  /// This should always return a value in [0,90[.
  /// For cylinders, this will return 0.
  double GetAngle() const;

protected:
  vtkConeFrustum(int res = 16);
  ~vtkConeFrustum() override;

  // int RequestInformation(vtkInformation* , vtkInformationVector** , vtkInformationVector* ) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  double BottomPoint[3]{ 0, 0, 0 };
  double BottomRadius{ 0.5 };
  double TopPoint[3]{ 0, 0, 1 };
  double TopRadius{ 0.0 };
  int Resolution;
  int OutputPointsPrecision{ SINGLE_PRECISION };
};

#endif
