//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshServer - Mesh representation for a vtkModel on the server.
// .SECTION Description

#ifndef __vtkCMBMeshServer_h
#define __vtkCMBMeshServer_h

#include "cmbSystemConfig.h"
#include "vtkCMBMesh.h"
#include <vtkSmartPointer.h> // for callback

class vtkCallbackCommand;
class vtkCMBMeshServerInternals;
class vtkCMBModelEntityMesh;
class vtkMergeEventData;
class vtkModel;
class vtkModelGeometricEntity;
class vtkSplitEventData;

class VTK_EXPORT vtkCMBMeshServer : public vtkCMBMesh
{
public:
  static vtkCMBMeshServer* New();
  vtkTypeMacro(vtkCMBMeshServer,vtkCMBMesh);
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
  virtual vtkCMBModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*);

protected:
  vtkCMBMeshServer();
  virtual ~vtkCMBMeshServer();

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
  vtkCMBMeshServer(const vtkCMBMeshServer&);  // Not implemented.
  void operator=(const vtkCMBMeshServer&);  // Not implemented.

  vtkCMBMeshServerInternals* Internal;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;
};

#endif
