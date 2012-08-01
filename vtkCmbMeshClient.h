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
// .NAME vtkCmbMeshClient - Mesh representation for a vtkModel on the client.
// .SECTION Description
// All meshing must be explicitly called on these entities (unless
// performing a model modification operation on an already meshed
// entity).  The client must call BuildModelEntityMeshes or
// the vtkCmbModelEntityMesh::BuildModelEntityMesh to explicitly
// generate a mesh on the server with the current meshing
// parameters.

#ifndef __vtkCmbMeshClient_h
#define __vtkCmbMeshClient_h

#include "vtkCmbMesh.h"
#include <vtkSmartPointer.h> // for callback

class vtkCallbackCommand;
class vtkCmbMeshClientInternals;
class vtkSMProxy;

class VTK_EXPORT vtkCmbMeshClient : public vtkCmbMesh
{
public:
  static vtkCmbMeshClient* New();
  vtkTypeMacro(vtkCmbMeshClient,vtkCmbMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  void Initialize(vtkModel* model, vtkSMProxy* serverModelProxy);

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

  virtual bool BuildModelEntityMeshes();

  virtual bool BuildModelMeshRepresentation(
    const char* fileName, const bool &isAnalysisMesh,
    vtkSMProxy* meshRepresentionInput=NULL);

  // Description:
  // Given a vtkModelGeometricEntity, get the associated mesh
  // representation.
  vtkCmbModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*);

  vtkGetObjectMacro(ServerModelProxy, vtkSMProxy);

  vtkSMProxy* GetServerMeshProxy()
  {return this->ServerMeshProxy;}

protected:
  vtkCmbMeshClient();
  virtual ~vtkCmbMeshClient();

  void SetServerModelProxy(vtkSMProxy*);

  // Description:
  // Process an edge split event from the model.  With a
  // constant edge length, the new edge gets the same length
  // size as the source edge.
  virtual void ModelEdgeSplit(vtkSplitEventData* splitEventData);

  // Description:
  // Process an edge merge event from the model.  With a constant
  // edge length, the surviving edge gets the smaller (valid) edge
  // length size.
  virtual void ModelEdgeMerge(vtkMergeEventData* mergeEventData);

  // Description:
  // If the boundary of an object changes, we will need to remesh it.
  // Note that if this is in conjunction with a split event, we won't
  // mesh it now but if it's in conjunction with a merge event we will.
  virtual void ModelEntityBoundaryModified(vtkModelGeometricEntity*);

private:
  vtkCmbMeshClient(const vtkCmbMeshClient&);  // Not implemented.
  void operator=(const vtkCmbMeshClient&);  // Not implemented.

  vtkSMProxy* ServerMeshProxy;
  vtkSMProxy* ServerModelProxy;
  vtkCmbMeshClientInternals* Internal;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;
};

#endif
