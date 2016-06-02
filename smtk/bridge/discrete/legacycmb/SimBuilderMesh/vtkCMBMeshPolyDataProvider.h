//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshPolyDataProvider - Algorithm for getting a polydata from CmbMesh
// .SECTION Description
// Given the vtkCMBMeshServer, a model entity Id and optionally an entity type,
// creates a filter output of the polydata used to represent the
// analysis mesh of the model entity.

#ifndef __vtkCMBMeshPolyDataProvider_h
#define __vtkCMBMeshPolyDataProvider_h

#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCMBMeshWrapper;
class vtkDataSet;
class vtkPolyData;

class VTK_EXPORT vtkCMBMeshPolyDataProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBMeshPolyDataProvider *New();
  vtkTypeMacro(vtkCMBMeshPolyDataProvider,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetMeshWrapper(vtkCMBMeshWrapper*);
  vtkGetMacro(MeshWrapper, vtkCMBMeshWrapper*);

  // Description:
  // Set/get functions to set the model item type.  This does not need
  // to be set in order to get the desired vtkPolyData for a model entity
  // but it makes the search faster.  If ItemType is set to
  // vtkModelType (0), then the master vtkPolyData is returned
  // in the output of the filter.
  void SetItemType(int itemType);
  vtkGetMacro(ItemType, int);

  // Description:
  // Set/get functions for the model entity id.  This does not need to be
  // set and is not used if ItemType is set to vtkModelType.
  void SetEntityId(vtkIdType Id);
  vtkGetMacro(EntityId, vtkIdType);

  // Description:
  // Setting controls whether or not to create vertex for model edges
  vtkBooleanMacro(CreateEdgePointVerts, bool);
  vtkSetMacro(CreateEdgePointVerts, bool);
  vtkGetMacro(CreateEdgePointVerts, bool);

protected:
  vtkCMBMeshPolyDataProvider();
  ~vtkCMBMeshPolyDataProvider();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

private:
  vtkCMBMeshPolyDataProvider(const vtkCMBMeshPolyDataProvider&);  // Not implemented.
  void operator=(const vtkCMBMeshPolyDataProvider&);  // Not implemented.

  vtkCMBMeshWrapper* MeshWrapper;
  int ItemType;
  bool ItemTypeIsSet;
  vtkIdType EntityId;
  bool EntityIdIsSet;
  bool CreateEdgePointVerts;

};

#endif
