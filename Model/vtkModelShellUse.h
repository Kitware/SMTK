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
// .NAME vtkModelShellUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkModelShellUse_h
#define __vtkModelShellUse_h

#include "vtkModelEntity.h"

class vtkModelFaceUse;
class vtkModelItemIterator;
class vtkModelRegion;

class VTK_EXPORT vtkModelShellUse : public vtkModelEntity
{
public:
  vtkTypeRevisionMacro(vtkModelShellUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  vtkModelItemIterator* NewModelFaceUseIterator();
  int GetNumberOfModelFaceUses();

  vtkModelRegion* GetModelRegion();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);
  
protected:
  static vtkModelShellUse* New();
  vtkModelShellUse();
  virtual ~vtkModelShellUse();

  // Description:
  // Remove a face use from this shell use.
  void RemoveModelFaceUse(vtkModelFaceUse* FaceUse);

  // Descrption:
  // Add the model face use to be adjacent to this shell use.
  // If FaceUse is currently adjacent to another face it
  // removes that association.
  void AddModelFaceUse(vtkModelFaceUse* FaceUse);

  virtual bool Destroy();
//BTX
  friend class vtkModelRegion;
  friend class vtkDiscreteModelFace;
  friend class vtkModelFaceUse;
  // these two friend classes should eventually come out
  // as they are specific to CMB
  friend class vtkSplitOperatorClient;
  friend class vtkSelectionSplitOperatorClient;
  friend class vtkDiscreteModelGeometricEntity;
//ETX

private:
  vtkModelShellUse(const vtkModelShellUse&);  // Not implemented.
  void operator=(const vtkModelShellUse&);  // Not implemented.
};

#endif

