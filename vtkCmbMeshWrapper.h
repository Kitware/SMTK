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
// .NAME vtkCmbMeshWrapper - Wrapper to get the vtkCmbMesh on the server.
// .SECTION Description
// Wrapper to get the vtkCmbMesh on the server.  A vtkSMProxyProperty is
// created on the client for this purpose.

#ifndef __vtkCmbMeshWrapper_h
#define __vtkCmbMeshWrapper_h

#include "vtkObject.h"

class vtkCmbMeshServer;
class vtkCMBModelWrapper;

class VTK_EXPORT vtkCmbMeshWrapper : public vtkObject
{
public:
  static vtkCmbMeshWrapper* New();
  vtkTypeRevisionMacro(vtkCmbMeshWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
  // Description:
  // Initialize vtkCmbMeshServer.
  void SetModelWrapper(vtkCMBModelWrapper*);
  vtkCmbMeshServer* GetMesh();
//ETX

  // Description:
  // Pass to actual mesh
  virtual void SetGlobalLength(double length);
  virtual void SetGlobalMaximumArea(double area);
  virtual void SetGlobalMinimumAngle(double angle);

protected:
  vtkCmbMeshWrapper();
  ~vtkCmbMeshWrapper();

private:
  vtkCmbMeshWrapper(const vtkCmbMeshWrapper&); // Not implemented
  void operator=(const vtkCmbMeshWrapper&); // Not implemented
//BTX
  vtkCmbMeshServer* Mesh;
//ETX
};

#endif
