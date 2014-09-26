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
// .NAME vtkCMBMeshWrapper - Wrapper to get the vtkCMBMesh on the server.
// .SECTION Description
// Wrapper to get the vtkCMBMesh on the server.  A vtkSMProxyProperty is
// created on the client for this purpose.

#ifndef __vtkCMBMeshWrapper_h
#define __vtkCMBMeshWrapper_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkCMBMeshServer;
class vtkDiscreteModelWrapper;

class VTK_EXPORT vtkCMBMeshWrapper : public vtkObject
{
public:
  static vtkCMBMeshWrapper* New();
  vtkTypeMacro(vtkCMBMeshWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize vtkCMBMeshServer.
  void SetModelWrapper(vtkDiscreteModelWrapper*);
//BTX
  vtkCMBMeshServer* GetMesh();
//ETX

  // Description:
  // Pass to actual mesh
  virtual void SetGlobalLength(double length);
  virtual void SetGlobalMinimumAngle(double angle);

protected:
  vtkCMBMeshWrapper();
  ~vtkCMBMeshWrapper();

private:
  vtkCMBMeshWrapper(const vtkCMBMeshWrapper&); // Not implemented
  void operator=(const vtkCMBMeshWrapper&); // Not implemented
  vtkCMBMeshServer* Mesh;

};

#endif
