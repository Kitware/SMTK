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
// .NAME vtkCMBModelFace - A model face based on a polydata representation.
// .SECTION Description

#ifndef __vtkCMBModelFace_h
#define __vtkCMBModelFace_h

#include "vtkModelFace.h"
#include "vtkCMBModelGeometricEntity.h"


class vtkCMBModelFaceUse;
class vtkIdList;
class vtkIdTypeArray;
class vtkBitArray;

class VTK_EXPORT vtkCMBModelFace : public vtkModelFace,
  public vtkCMBModelGeometricEntity
{
public:
  vtkTypeRevisionMacro(vtkCMBModelFace,vtkModelFace);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkCMBModelFace *New();

  // Description:
  // Split this model face based on SplitAngle.  The function
  // fills the created model face UniquePersistentId in
  // CreatedModelFace, and returns true if successful.
  bool Split(double SplitAngle, vtkIdTypeArray* CreatedModelFace);

  // Description:
  // Get All/Boundary/Interior point Ids of this model face.
  virtual void GetAllPointIds(vtkIdList* ptsList);
  virtual void GetInteriorPointIds(vtkIdList* ptsList);
  virtual void GetBoundaryPointIds(vtkIdList* ptsList);

  // Description:
  // Mark each index in PointsMask with 0 (out) or 1 (in) for
  // master vtkPoints that are in the vtkCMBModelFace grid.
  void GatherAllPointIdsMask(vtkBitArray* PointsMask);

  // Description:
  // Get All/Boundary/Interior point Ids of this model face.
  // Mark each index in PointsMask with 0 (out) or 1 (in) for
  // master vtkPoints that are on the boundary of vtkCMBModelFace grid.
  void GatherBoundaryPointIdsMask(vtkBitArray* Points);

protected:
//BTX
  friend class vtkDiscreteModel;
  friend class vtkCmbMapToCmbModel;
  friend class vtkCmbBCGridRepresentation;
//ETX
  vtkCMBModelFace();
  virtual ~vtkCMBModelFace();

  // Description:
  // Build a new model face from the cells listed in CellIds.
  // The Ids listed in CellIds are with respect to the master grid.
  vtkCMBModelFace* BuildFromExistingModelFace(vtkIdList* CellIds);
  friend class vtkSelectionSplitOperator;
  friend class vtkCmbIncorporateMeshOperator;

  virtual vtkModelEntity* GetThisModelEntity();
  virtual bool Destroy();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

private:
  vtkCMBModelFace(const vtkCMBModelFace&);  // Not implemented.
  void operator=(const vtkCMBModelFace&);  // Not implemented.

  void CreateModelFaceUses();
};

#endif

