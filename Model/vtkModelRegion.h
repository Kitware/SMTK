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
// .NAME vtkModelRegion - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkModelRegion_h
#define __vtkModelRegion_h

#include "vtkModelGeometricEntity.h"

class vtkIdList;
class vtkModelItemIterator;
class vtkModelFace;
class vtkModelFaceUse;
class vtkModelShellUse;

class VTK_EXPORT vtkModelRegion : public vtkModelGeometricEntity
{
public:
  vtkTypeRevisionMacro(vtkModelRegion,vtkModelGeometricEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkModelItemIterator* NewModelShellUseIterator();

  virtual int GetType();

  virtual void Initialize(int NumModelFaces, vtkModelFace** Faces, int* FaceSides,
                          vtkIdType ModelRegionId);

  virtual void Initialize(vtkIdType ModelRegionId);

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
  
  virtual void AddShell(int NumFaces, vtkModelFace** Faces,
                        int* FaceSides);
protected:
  vtkModelRegion();
  virtual ~vtkModelRegion();

  virtual vtkModelShellUse* BuildModelShellUse(
    int NumModelFaces, vtkModelFace** Faces, int* FaceSides);
  virtual bool DestroyModelShellUse(vtkModelShellUse* ShellUse);
  
  virtual bool IsDestroyable();
  virtual bool Destroy();
//BTX
  friend class vtkModel;
//ETX

private:
  vtkModelRegion(const vtkModelRegion&);  // Not implemented.
  void operator=(const vtkModelRegion&);  // Not implemented.
};

#endif

