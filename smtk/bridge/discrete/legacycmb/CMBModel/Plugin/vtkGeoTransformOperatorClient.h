//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGeoTransformOperatorClient - The client side object of
//       vtkGeoTransformOperator
// .SECTION Description

#ifndef __vtkGeoTransformOperatorClient_h
#define __vtkGeoTransformOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkSMProxy;
class vtkSMOperatorProxy;

class VTK_EXPORT vtkGeoTransformOperatorClient : public vtkObject
{
public:
  static vtkGeoTransformOperatorClient* New();
  vtkTypeMacro(vtkGeoTransformOperatorClient, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  vtkGeoTransformOperatorClient();
  virtual ~vtkGeoTransformOperatorClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  bool ConvertFromLatLongToXYZ;

private:
  vtkGeoTransformOperatorClient(const vtkGeoTransformOperatorClient&); // Not implemented.
  void operator=(const vtkGeoTransformOperatorClient&);                // Not implemented.
  vtkSMOperatorProxy* OperatorProxy;
};

#endif
