//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelRegion - Abstract generic model entity class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelRegion_h
#define __smtkdiscrete_vtkModelRegion_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGeometricEntity.h"

class vtkIdList;
class vtkModelItemIterator;
class vtkModelFace;
class vtkModelFaceUse;
class vtkModelShellUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelRegion : public vtkModelGeometricEntity
{
public:
  vtkTypeMacro(vtkModelRegion, vtkModelGeometricEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkModelItemIterator* NewModelShellUseIterator();

  virtual int GetType();

  virtual void Initialize(
    int numModelFaces, vtkModelFace** faces, int* faceSides, vtkIdType modelRegionId);

  virtual void Initialize(vtkIdType modelRegionId);

  // Description:
  // Get the number of shells the region has
  int GetNumberOfShells();

  // Description:
  // Get the number of faces the region has
  int GetNumberOfFaces();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Return an iterator to access the model faces that define
  // the boundaries of this model region.
  vtkModelItemIterator* NewAdjacentModelFaceIterator();

  virtual void AddShell(int numFaces, vtkModelFace** faces, int* faceSides);

protected:
  vtkModelRegion();
  virtual ~vtkModelRegion();

  virtual vtkModelShellUse* BuildModelShellUse(
    int numModelFaces, vtkModelFace** faces, int* faceSides);
  virtual bool DestroyModelShellUse(vtkModelShellUse* shellUse);

  virtual bool IsDestroyable();
  virtual bool Destroy();

  friend class vtkModel;

private:
  vtkModelRegion(const vtkModelRegion&); // Not implemented.
  void operator=(const vtkModelRegion&); // Not implemented.
};

#endif
