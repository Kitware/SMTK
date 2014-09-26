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
// .NAME vtkCMBMeshGridRepresentationOperator
// .SECTION Description
// Operator that reads in the simulation mesh on the server and creates a
// vtkCMBMeshGridRepresentation to be used by the model.

#ifndef __vtkCMBMeshGridRepresentationOperator_h
#define __vtkCMBMeshGridRepresentationOperator_h

#include "vtkObject.h"
#include "vtkWeakPointer.h"
#include "cmbSystemConfig.h"

class vtkCMBMeshWrapper;
class vtkAlgorithm;

class VTK_EXPORT vtkCMBMeshGridRepresentationOperator : public vtkObject
{
public:
  static vtkCMBMeshGridRepresentationOperator * New();
  vtkTypeMacro(vtkCMBMeshGridRepresentationOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets OperateSucceeded.
  void Operate(vtkCMBMeshWrapper* meshWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Set if the mesh should be used as the simultation input mesh.
  vtkGetMacro(MeshIsAnalysisGrid, int);
  vtkSetMacro(MeshIsAnalysisGrid, int);

  // Description:
  // Set the file name to save the mesh out too.
  // Note: Setting the file name will cause the mesh to be written out
  vtkGetStringMacro(GridFileName);
  vtkSetStringMacro(GridFileName);

  // Description:
  // Source proxy for mesh representation. Default is null, and the mesh representation
  // will be built later in vtkCMBMeshGridRepresentionServer. However, if this is set,
  // it will be passed to vtkCMBMeshGridRepresentionServer as its mesh representation.
  void SetMeshRepresentationInput(vtkAlgorithm* meshSource);

protected:
  vtkCMBMeshGridRepresentationOperator();
  virtual ~vtkCMBMeshGridRepresentationOperator();

private:

  vtkCMBMeshGridRepresentationOperator(const vtkCMBMeshGridRepresentationOperator&);  // Not implemented.
  void operator=(const vtkCMBMeshGridRepresentationOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // Flag to indicate that after saving the mesh to file we also want to set this mesh
  // as the grid info representation
  int MeshIsAnalysisGrid;

  char* GridFileName;
  vtkWeakPointer<vtkAlgorithm> MeshRepresentationSource;
};

#endif
