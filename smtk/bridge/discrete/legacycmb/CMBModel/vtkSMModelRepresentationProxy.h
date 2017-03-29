//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSMModelRepresentationProxy - a composite representation proxy suitable
// for showing a cmb model
// .SECTION Description

#ifndef __vtkSMModelRepresentationProxy_h
#define __vtkSMModelRepresentationProxy_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSMPVRepresentationProxy.h"

class VTKCMBDISCRETEMODEL_EXPORT vtkSMModelRepresentationProxy : public vtkSMPVRepresentationProxy
{
public:
  static vtkSMModelRepresentationProxy* New();
  vtkTypeMacro(vtkSMModelRepresentationProxy, vtkSMPVRepresentationProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the type of representation.
  virtual void SetRepresentation(int type);

  enum ModelRepresentationType
    {
    POINTS=0,
    WIREFRAME=1,
    SURFACE=2,
    SURFACE_WITH_EDGES=3,
    USER_DEFINED=100,
    // Special identifiers for back faces.
    FOLLOW_FRONTFACE=400,
    CULL_BACKFACE=401,
    CULL_FRONTFACE=402
    };

protected:
  vtkSMModelRepresentationProxy();
  ~vtkSMModelRepresentationProxy();

  // Description:
  virtual void CreateVTKObjects();
  int Representation;

private:
  vtkSMModelRepresentationProxy(const vtkSMModelRepresentationProxy&); // Not implemented
  void operator=(const vtkSMModelRepresentationProxy&); // Not implemented

};

#endif
