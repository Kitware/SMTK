//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSMModelProxy
// .SECTION Description
//

#ifndef __vtkCMBSMModelProxy_h
#define __vtkCMBSMModelProxy_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSMProxy.h"

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBSMModelProxy : public vtkSMProxy
{
public:
  static vtkCMBSMModelProxy* New();
  vtkTypeMacro(vtkCMBSMModelProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  void Refresh();

protected:
  vtkCMBSMModelProxy();
  ~vtkCMBSMModelProxy();

private:
  vtkCMBSMModelProxy(const vtkCMBSMModelProxy&); // Not implemented
  void operator=(const vtkCMBSMModelProxy&);     // Not implemented
};

#endif
