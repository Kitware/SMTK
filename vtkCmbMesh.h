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
// .NAME vtkCmbMesh - Abstract mesh representation for a vtkModel.
// .SECTION Description
// Both the client and server mesh representation derive from this.

#ifndef __vtkCmbMesh_h
#define __vtkCmbMesh_h

#include <vtkObject.h>
#include <vtkWeakPointer.h>

class vtkCmbModelEntityMesh;
class vtkCollection;
class vtkMergeEventData;
class vtkModel;
class vtkModelGeometricEntity;
class vtkSplitEventData;

class VTK_EXPORT vtkCmbMesh : public vtkObject
{
public:
  vtkTypeMacro(vtkCmbMesh,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get visible (non-zero is visible).
  vtkSetMacro(Visible, bool);
  vtkGetMacro(Visible, bool);

  // Description:
  // The absolute length set over all arcs/model edges.
  // If GlobalLength is less than or equal to zero, it
  // is still unset.
  virtual bool SetGlobalLength(double length) = 0;
  vtkGetMacro(GlobalLength, double);

  // Description:
  // The global minimum angle allowed for surface elements.
  // If GlobalMinimumAngle is less than or equal to zero, it
  // is still unset.  The maximum value for minimum angle
  // is 33 as enforced by triangle.
  virtual bool SetGlobalMinimumAngle(double angle) = 0;
  vtkGetMacro(GlobalMinimumAngle, double);

  virtual void Reset();

  // Description:
  // Given a vtkModelGeometricEntity, get the associated mesh
  // representation.
  virtual vtkCmbModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*) =0;

  // Description:
  // Set the local mesh length/max area/min angle for those selected model entities
  bool SetLocalMeshLength(
    vtkCollection* selectedMeshEntities, double localLen);
  bool SetLocalMeshMinimumAngle(
    vtkCollection* selectedMeshEntities, double localMinAngle);

  vtkModel* GetModel();

  // Description:
  // Given 2 mesh lengths return the combine length.
  static double CombineMeshLengths(double a, double b);
  // Description:
  // Given 2 mesh min angles return the combine angle.
  static double CombineMeshMinimumAngles(double a, double b);

protected:
  vtkCmbMesh();
  virtual ~vtkCmbMesh();

  // Description:
  // Main callback function which delegates the work to specific
  // methods for model entity modifications.
  static void ModelGeometricEntityChanged(
    vtkObject *caller, unsigned long event,
    void *cData, void *callData);

  // Description:
  // Process an edge split event from the model.  With a
  // constant edge length, the new edge gets the same length
  // size as the source edge.
  virtual void ModelEdgeSplit(vtkSplitEventData* splitEventData) = 0;

  // Description:
  // Process an edge merge event from the model.  With a constant
  // edge length, the surviving edge gets the smaller (valid) edge
  // length size.
  virtual void ModelEdgeMerge(vtkMergeEventData* mergeEventData) = 0;

  // Description:
  // If the boundary of an object changes, we will need to remesh it.
  // Note that if this is in conjunction with a split event, we won't
  // mesh it now but if it's in conjunction with a merge event we will.
  virtual void ModelEntityBoundaryModified(vtkModelGeometricEntity*) = 0;

  double GlobalLength;
  double GlobalMinimumAngle;
  vtkWeakPointer<vtkModel> Model;

private:
  vtkCmbMesh(const vtkCmbMesh&);  // Not implemented.
  void operator=(const vtkCmbMesh&);  // Not implemented.

  bool Visible;
};

// Return the smaller non-zero length
inline  double vtkCmbMesh::CombineMeshLengths(double a, double b)
{
  if (a == 0.0)
    {
    return b;
    }
  if (b == 0.0)
    {
    return a;
    }
  return (a > b) ? b : a;
}

// Return the larger area
inline double vtkCmbMesh::CombineMeshMinimumAngles(double a, double b)
{
  return (a > b) ? a : b;
}

#endif

