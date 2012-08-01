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
// .NAME vtkCmbMeshServer - Mesh representation for a vtkModel on the server.
// .SECTION Description

#ifndef __vtkCmbMeshServer_h
#define __vtkCmbMeshServer_h

#include "vtkCmbMesh.h"
#include <vtkSmartPointer.h> // for callback

class vtkCallbackCommand;
class vtkCmbMeshServerInternals;
class vtkCmbModelEntityMesh;
class vtkMergeEventData;
class vtkModel;
class vtkModelGeometricEntity;
class vtkSplitEventData;

class VTK_EXPORT vtkCmbMeshServer : public vtkCmbMesh
{
public:
  static vtkCmbMeshServer* New();
  vtkTypeMacro(vtkCmbMeshServer,vtkCmbMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(vtkModel* model);

  // Description:
  // The absolute length set over all arcs/model edges.
  // If GlobalLength is less than or equal to zero, it
  // is still unset.
  virtual bool SetGlobalLength(double length);

  // Description:
  // The global minimum angle allowed for surface elements.
  // If GlobalMinimumAngle is less than or equal to zero, it
  // is still unset.  The maximum value for minimum angle
  // is 33 as enforced by triangle.
  virtual bool SetGlobalMinimumAngle(double angle);

  virtual void Reset();

  // Description:
  // Given a vtkModelGeometricEntity, get the associated mesh
  // representation.
  virtual vtkCmbModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*);

protected:
  vtkCmbMeshServer();
  virtual ~vtkCmbMeshServer();

  // Description:
  // Process an edge split event from the model.  With a
  // constant edge length, the new edge gets the same length
  // size as the source edge.
  void ModelEdgeSplit(vtkSplitEventData* splitEventData);

  // Description:
  // Process an edge merge event from the model.  With a constant
  // edge length, the surviving edge gets the smaller (valid) edge
  // length size.
  void ModelEdgeMerge(vtkMergeEventData* mergeEventData);

  // Description:
  // If the boundary of an object changes, we will need to remesh it.
  // Note that if this is in conjunction with a split event, we won't
  // mesh it now but if it's in conjunction with a merge event we will.
  void ModelEntityBoundaryModified(vtkModelGeometricEntity*);

private:
  vtkCmbMeshServer(const vtkCmbMeshServer&);  // Not implemented.
  void operator=(const vtkCmbMeshServer&);  // Not implemented.

  vtkCmbMeshServerInternals* Internal;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;
};

#endif
