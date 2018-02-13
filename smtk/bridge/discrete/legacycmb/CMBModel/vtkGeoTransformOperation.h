//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGeoTransformOperation - A class for converting lat-long to XYZ
// .SECTION Description

#ifndef __vtkGeoTransformOperation_h
#define __vtkGeoTransformOperation_h

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

class VTKCMBDISCRETEMODEL_EXPORT vtkGeoTransformOperation : public vtkObject
{
public:
  static vtkGeoTransformOperation* New();
  vtkTypeMacro(vtkGeoTransformOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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
  vtkGeoTransformOperation();
  virtual ~vtkGeoTransformOperation();

private:
  // Description:
  // Internal ivars.
  vtkGeoTransformOperation(const vtkGeoTransformOperation&); // Not implemented.
  void operator=(const vtkGeoTransformOperation&);           // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  bool ConvertFromLatLongToXYZ;
  vtkSmartPointer<vtkGeoSphereTransform> LatLongTransform1;
  vtkSmartPointer<vtkTransform> LatLongTransform2;
  vtkSmartPointer<vtkPoints> OriginalModelPoints;
};

#endif
