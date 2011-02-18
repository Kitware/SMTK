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

#ifndef __vtkCmbMeshClient_h
#define __vtkCmbMeshClient_h

#include "vtkCmbMesh.h"

class vtkCmbMeshClientInternals;
class vtkSMProxy;
class vtkCollection;

class VTK_EXPORT vtkCmbMeshClient : public vtkCmbMesh
{
public:
  static vtkCmbMeshClient* New();
  vtkTypeRevisionMacro(vtkCmbMeshClient,vtkCmbMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  void Initialize(vtkModel* model, vtkSMProxy* serverModelProxy);

  // Description:
  // The absolute length set over all arcs/model edges.
  // If GlobalLength is less than or equal to zero, it
  // is still unset.
  virtual void SetGlobalLength(double length);

  // Description:
  // The absolute maximum area set over all model faces.
  // If GlobalMaximumArea is less than or equal to zero, it
  // is still unset.
  virtual void SetGlobalMaximumArea(double area);

  // Description:
  // The global minimum angle allowed for surface elements.
  // If GlobalMinimumAngle is less than or equal to zero, it
  // is still unset.
  virtual void SetGlobalMinimumAngle(double angle);

  // Description:
  // Set the local mesh length for those selected model entities
  bool SetLocalMeshLength(
    vtkCollection* selectedMeshEntities,
    double localLen, bool meshHigherDimensionalEntities=false);

  virtual void Reset();

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
};

#endif
