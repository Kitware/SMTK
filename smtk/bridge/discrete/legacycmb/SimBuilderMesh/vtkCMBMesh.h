//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMesh - Abstract mesh representation for a vtkModel.
// .SECTION Description
// Both the client and server mesh representation derive from this.

#ifndef __vtkCMBMesh_h
#define __vtkCMBMesh_h

#include "cmbSystemConfig.h"
#include <vtkObject.h>
#include <vtkWeakPointer.h>

class vtkCMBModelEntityMesh;
class vtkCollection;
class vtkMergeEventData;
class vtkModel;
class vtkModelGeometricEntity;
class vtkSplitEventData;

class VTK_EXPORT vtkCMBMesh : public vtkObject
{
public:
  vtkTypeMacro(vtkCMBMesh, vtkObject);
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
  virtual vtkCMBModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*) = 0;

  // Description:
  // Set the local mesh length/max area/min angle for those selected model entities
  bool SetLocalMeshLength(vtkCollection* selectedMeshEntities, double localLen);
  bool SetLocalMeshMinimumAngle(vtkCollection* selectedMeshEntities, double localMinAngle);

  vtkModel* GetModel();

  // Description:
  // Given 2 mesh lengths return the combine length.
  static double CombineMeshLengths(double a, double b);
  // Description:
  // Given 2 mesh min angles return the combine angle.
  static double CombineMeshMinimumAngles(double a, double b);

protected:
  vtkCMBMesh();
  virtual ~vtkCMBMesh();

  // Description:
  // Main callback function which delegates the work to specific
  // methods for model entity modifications.
  static void ModelGeometricEntityChanged(
    vtkObject* caller, unsigned long event, void* cData, void* callData);

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
  vtkCMBMesh(const vtkCMBMesh&);     // Not implemented.
  void operator=(const vtkCMBMesh&); // Not implemented.

  bool Visible;
};

// Return the smaller non-zero length
inline double vtkCMBMesh::CombineMeshLengths(double a, double b)
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
inline double vtkCMBMesh::CombineMeshMinimumAngles(double a, double b)
{
  return (a > b) ? a : b;
}

#endif
