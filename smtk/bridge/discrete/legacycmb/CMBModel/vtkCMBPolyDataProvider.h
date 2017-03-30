//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBPolyDataProvider - Algorithm for getting a polydata from a CMB Model
// .SECTION Description
// Given the CMB model, a model entity Id and optionally an entity type,
// creates a filter output of the polydata used to represent the
// model entity.

#ifndef __vtkCMBPolyDataProvider_h
#define __vtkCMBPolyDataProvider_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkDiscreteModelWrapper;
class vtkDataSet;
class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBPolyDataProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBPolyDataProvider* New();
  vtkTypeMacro(vtkCMBPolyDataProvider, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetModelWrapper(vtkDiscreteModelWrapper*);
  vtkGetMacro(ModelWrapper, vtkDiscreteModelWrapper*);

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
  vtkCMBPolyDataProvider();
  ~vtkCMBPolyDataProvider();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

private:
  vtkCMBPolyDataProvider(const vtkCMBPolyDataProvider&); // Not implemented.
  void operator=(const vtkCMBPolyDataProvider&);         // Not implemented.

  vtkDiscreteModelWrapper* ModelWrapper;
  int ItemType;
  bool ItemTypeIsSet;
  vtkIdType EntityId;
  bool EntityIdIsSet;
  bool CreateEdgePointVerts;
};

#endif
