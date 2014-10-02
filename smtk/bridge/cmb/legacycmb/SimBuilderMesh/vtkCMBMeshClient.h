//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshClient - Mesh representation for a vtkModel on the client.
// .SECTION Description
// All meshing must be explicitly called on these entities (unless
// performing a model modification operation on an already meshed
// entity).  The client must call BuildModelEntityMeshes or
// the vtkCMBModelEntityMesh::BuildModelEntityMesh to explicitly
// generate a mesh on the server with the current meshing
// parameters.

#ifndef __vtkCMBMeshClient_h
#define __vtkCMBMeshClient_h

#include "vtkCMBMesh.h"
#include <vtkSmartPointer.h> // for callback
#include "cmbSystemConfig.h"

class vtkCallbackCommand;
class vtkCMBMeshClientInternals;
class vtkSMProxy;

class VTK_EXPORT vtkCMBMeshClient : public vtkCMBMesh
{
public:
  static vtkCMBMeshClient* New();
  vtkTypeMacro(vtkCMBMeshClient,vtkCMBMesh);
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
  vtkCMBModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*);

  vtkGetObjectMacro(ServerModelProxy, vtkSMProxy);

  vtkSMProxy* GetServerMeshProxy()
  {return this->ServerMeshProxy;}

protected:
  vtkCMBMeshClient();
  virtual ~vtkCMBMeshClient();

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
  vtkCMBMeshClient(const vtkCMBMeshClient&);  // Not implemented.
  void operator=(const vtkCMBMeshClient&);  // Not implemented.

  vtkSMProxy* ServerMeshProxy;
  vtkSMProxy* ServerModelProxy;
  vtkCMBMeshClientInternals* Internal;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;
};

#endif
