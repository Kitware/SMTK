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
// .NAME vtkCMBModelOmicronMeshInputWriterClient -
// .SECTION Description


#ifndef __vtkCMBModelOmicronMeshInputWriterClient_h
#define __vtkCMBModelOmicronMeshInputWriterClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelOmicronMeshInputWriterClient : public vtkObject
{
public:
  static vtkCMBModelOmicronMeshInputWriterClient * New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriterClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set the filename of the geometry file associated with this file.
  vtkSetStringMacro(GeometryFileName);
  vtkGetStringMacro(GeometryFileName);

  // Description:
  // Get/Set the TetGen Options.
  vtkSetStringMacro(TetGenOptions);
  vtkGetStringMacro(TetGenOptions);

  // Description:
  // Get/Set the volume constraint
  vtkSetMacro(VolumeConstraint, double);
  vtkGetMacro(VolumeConstraint, double);

protected:
  vtkCMBModelOmicronMeshInputWriterClient();
  virtual ~vtkCMBModelOmicronMeshInputWriterClient();

private:
  vtkCMBModelOmicronMeshInputWriterClient(const vtkCMBModelOmicronMeshInputWriterClient&);  // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriterClient&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  char* GeometryFileName;
  char* TetGenOptions;
  double VolumeConstraint;
};

#endif
