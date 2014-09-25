/*=========================================================================

  Program:   ParaView
  Module:    vtkSIModelRepresentationProxy.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSIModelRepresentationProxy - a composite representation proxy suitable
// for showing a cmb model
// .SECTION Description

#ifndef __vtkSIModelRepresentationProxy_h
#define __vtkSIModelRepresentationProxy_h

#include "vtkSIPVRepresentationProxy.h"
#include "cmbSystemConfig.h"

class vtkSIModelRepresentationProxy : public vtkSIPVRepresentationProxy
{
public:
  static vtkSIModelRepresentationProxy* New();
  vtkTypeMacro(vtkSIModelRepresentationProxy, vtkSIPVRepresentationProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSIModelRepresentationProxy();
  ~vtkSIModelRepresentationProxy();

private:
  vtkSIModelRepresentationProxy(const vtkSIModelRepresentationProxy&); // Not implemented
  void operator=(const vtkSIModelRepresentationProxy&); // Not implemented

};

#endif