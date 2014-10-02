/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkGeoTransformOperator - A class for converting lat-long to XYZ
// .SECTION Description

#ifndef __vtkGeoTransformOperator_h
#define __vtkGeoTransformOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

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
