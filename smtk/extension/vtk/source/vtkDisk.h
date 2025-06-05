//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkDisk_h
#define vtkDisk_h

#include "smtk/extension/vtk/source/vtkImplicitDisk.h" // For ivar
#include "vtkPolyDataAlgorithm.h"

#include "vtkCell.h" // Needed for VTK_CELL_SIZE

/**
 * @class   vtkDisk
 * @brief   Generate a polygonal approximation to a disk
 *
 * vtkDisk creates a disk with a given center, normal, and radius.
 * By default, point 1 lies at the origin with radius 0.5
 * and the normal is (0,0,1).
 * The resolution specifies the number of points around the circle;
 * it must be at least 3 and defaults to 32.
 *
 * Note that unlike many other source filters, this one is *not*
 * intended for use as an input to a glyph filter or glyph mapper.
 * Instead, its purpose is to provide a high-quality visual representation
 * of a planar disk.
 * That means it uses more points and cells than is strictly necessary
 * but is able to provide a better visual quality as a result.
 */
class VTKSMTKSOURCEEXT_EXPORT vtkDisk : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkDisk, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkDisk(const vtkDisk&) = delete;
  vtkDisk& operator=(const vtkDisk&) = delete;

  /// An enum indexing data present at each output.
  enum OutputPorts
  {
    DiskFace = 0, //!< Triangulation of the disk face.
    DiskNormal,   //!< Line from the center point along the normal vector (length of disk radius).
    DiskEdge,     //!< Polyline of the disk boundary.
    CenterVertex, //!< A single vertex at the center of the bottom face.
    NumberOfOutputs
  };

  /**
   * Construct with default parameters.
   */
  static vtkDisk* New();

  //@{
  /**
   * Set/get the radius at the bottom of the cone.
   *
   * It must be non-negative.
   */
  vtkSetClampMacro(Radius, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(Radius, double);
  //@}

  //@{
  /**
   * Set/get the bottom point of the cone.
   * The default is 0,0,0.
   */
  vtkSetVector3Macro(CenterPoint, double);
  vtkGetVectorMacro(CenterPoint, double, 3);
  //@}

  //@{
  /**
   * Set/get the normal to the disk's plane.
   * The default is 0,0,1.
   */
  vtkSetVector3Macro(Normal, double);
  vtkGetVectorMacro(Normal, double, 3);
  //@}

  //@{
  /**
   * Set/get the number of facets used to represent the conical side-face.
   *
   * This defaults to 32 and has a minimum of 3.
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

protected:
  vtkDisk(int res = 32);
  ~vtkDisk() override;

  // int RequestInformation(vtkInformation* , vtkInformationVector** , vtkInformationVector* ) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  double CenterPoint[3]{ 0, 0, 0 };
  double Radius{ 0.5 };
  double Normal[3]{ 0, 0, 1 };
  int Resolution;
  int OutputPointsPrecision{ SINGLE_PRECISION };
};

#endif
