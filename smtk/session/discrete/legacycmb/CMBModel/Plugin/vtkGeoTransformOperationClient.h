//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGeoTransformOperationClient - The client side object of
//       vtkGeoTransformOperation
// .SECTION Description

#ifndef __vtkGeoTransformOperationClient_h
#define __vtkGeoTransformOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;
class vtkSMOperationProxy;

class VTK_EXPORT vtkGeoTransformOperationClient : public vtkObject
{
public:
  static vtkGeoTransformOperationClient* New();
  vtkTypeMacro(vtkGeoTransformOperationClient, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Using the input poly on the server to set up the modle
  // and serializes the model to the client.
  // Returns true if the operation was successful.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  vtkBooleanMacro(ConvertFromLatLongToXYZ, bool);
  vtkSetMacro(ConvertFromLatLongToXYZ, bool);
  vtkGetMacro(ConvertFromLatLongToXYZ, bool);

protected:
  vtkGeoTransformOperationClient();
  virtual ~vtkGeoTransformOperationClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  bool ConvertFromLatLongToXYZ;

private:
  vtkGeoTransformOperationClient(const vtkGeoTransformOperationClient&); // Not implemented.
  void operator=(const vtkGeoTransformOperationClient&);                 // Not implemented.
  vtkSMOperationProxy* OperationProxy;
};

#endif
