//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelShellUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelShellUse_h
#define __smtkdiscrete_vtkModelShellUse_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"

class vtkModelFaceUse;
class vtkModelItemIterator;
class vtkModelRegion;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelShellUse : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelShellUse, vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  int GetType() override;

  vtkModelItemIterator* NewModelFaceUseIterator();
  int GetNumberOfModelFaceUses();

  vtkModelRegion* GetModelRegion();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  void Serialize(vtkSerializer* ser) override;

protected:
  static vtkModelShellUse* New();
  vtkModelShellUse();
  ~vtkModelShellUse() override;

  // Description:
  // Remove a face use from this shell use.
  void RemoveModelFaceUse(vtkModelFaceUse* faceUse);

  // Descrption:
  // Add the model face use to be adjacent to this shell use.
  // If FaceUse is currently adjacent to another face it
  // removes that association.
  void AddModelFaceUse(vtkModelFaceUse* faceUse);

  bool Destroy() override;

  friend class vtkModelRegion;
  friend class vtkDiscreteModelFace;
  friend class vtkModelFaceUse;
  // these two friend classes should eventually come out
  // as they are specific to CMB
  friend class vtkSplitOperationClient;
  friend class vtkSelectionSplitOperationClient;
  friend class vtkDiscreteModelGeometricEntity;

private:
  vtkModelShellUse(const vtkModelShellUse&); // Not implemented.
  void operator=(const vtkModelShellUse&);   // Not implemented.
};

#endif
