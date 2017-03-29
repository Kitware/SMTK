//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGeoTransformOperator - A class for converting lat-long to XYZ
// .SECTION Description

#ifndef __vtkGeoTransformOperator_h
#define __vtkGeoTransformOperator_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "vtkSmartPointer.h"

class vtkCellLocator;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkAlgorithm;
class vtkTransform;
class vtkGeoSphereTransform;
class vtkPoints;

class VTKCMBDISCRETEMODEL_EXPORT vtkGeoTransformOperator : public vtkObject
{
public:
  static vtkGeoTransformOperator * New();
  vtkTypeMacro(vtkGeoTransformOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the input polydata into Model.
  void Operate(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  vtkBooleanMacro(ConvertFromLatLongToXYZ, bool);
  vtkSetMacro(ConvertFromLatLongToXYZ, bool);
  vtkGetMacro(ConvertFromLatLongToXYZ, bool);

protected:
  vtkGeoTransformOperator();
  virtual ~vtkGeoTransformOperator();

private:

  // Description:
  // Internal ivars.
  vtkGeoTransformOperator(const vtkGeoTransformOperator&);  // Not implemented.
  void operator=(const vtkGeoTransformOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  bool ConvertFromLatLongToXYZ;
  vtkSmartPointer<vtkGeoSphereTransform> LatLongTransform1;
  vtkSmartPointer<vtkTransform> LatLongTransform2;
  vtkSmartPointer<vtkPoints> OriginalModelPoints;

};

#endif
