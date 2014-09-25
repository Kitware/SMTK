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
// .NAME vtkCMBModelOmicronMeshInputWriterBase -
// .SECTION Description

#ifndef __vtkCMBModelOmicronMeshInputWriterBase_h
#define __vtkCMBModelOmicronMeshInputWriterBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

//class vtkDiscreteModel;
//class vtkDiscreteModelGeometricEntity;
class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelOmicronMeshInputWriterBase : public vtkObject
{
public:
  static vtkCMBModelOmicronMeshInputWriterBase * New();
  vtkTypeMacro(vtkCMBModelOmicronMeshInputWriterBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Get/Set the name of the output file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set the name of the geometry file.
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

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelOmicronMeshInputWriterBase();
  virtual ~vtkCMBModelOmicronMeshInputWriterBase();

private:
  vtkCMBModelOmicronMeshInputWriterBase(const vtkCMBModelOmicronMeshInputWriterBase&);  // Not implemented.
  void operator=(const vtkCMBModelOmicronMeshInputWriterBase&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  // Description:
  // The name of the geoemtry file to be listed in the output file.
  char* GeometryFileName;

  // Description:
  // The name of the TetGen Options to be given to the mesher.
  char* TetGenOptions;

  // Description:
  // The volume constraint to pass to the mesher.
  double VolumeConstraint;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
